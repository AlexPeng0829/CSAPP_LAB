/*
* Explict free list strategy
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

team_t team = {
    "Real Madrid",
    "Alex Peng",
    "",
    "",
    ""};

// clang-format off
#define DEBUG 0
#define __M64 1

#define ALIGN                       8
#define CHUNKSIZE                   (1 << 12)

#define D_PRINT                     DEBUG && printf


#if __M64
#define WSIZE                       8
#define QSIZE                       8*WSIZE
#define address_len                 uint64_t
#else
#define WSIZE                       4
#define QSIZE                       4*WSIZE
#define address_len                 uint32_t
#endif

#define PACK(size, alloc)           ((size) | (alloc))
#define GET(p)                      (*(address_len *)(p))
#define PUT(p, val)                 (*(address_len *)(p) = (val))
#define GET_SIZE(p)                 (GET(p) & ~0x7)
#define GET_ALLOC(p)                (GET(p) & 0x1)

// predecessor and successor in explicit linked-list
#define PRED(p)                     (*(address_len *)(p))
#define SUCC(p)                     (*(address_len *)((void *)(p) + WSIZE))

// ajacent block in address, footer or header
#define AJACENT_PREV_FOOTER(p)      ((void *)(p) - 2*WSIZE)
#define AJACENT_NEXT_HEADER(p)      ((void *)(p) + GET_SIZE(HDRP(p)) - WSIZE)

// IMPORTANT! header must be updated first so that the size is properly updated!
#define HDRP(payload)               ((void *)(payload) - WSIZE)
#define FTRP(payload)               ((void *)(payload) + GET_SIZE(HDRP(payload)) - 2*WSIZE)

// this is used for colored output(funny face)
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
// clang-format on

/*
*  Explict-empty-list

                  upper_addresss
            ========================
            |FOOTER   size | 2 1 0 |
            |----------------------|
            |(optional)    Padding |
            |----------------------|
            |SUCC       |          |
            |---------- |  Payload |
     ptr -->|PRED       |          |
            |----------------------|
            |HEADER   size | 2 1 0 |
            ========================
                  lower_addresss

*NOTE that the explicit free list is always ordered by the address from lower to higher. To achieve that, we need to
* insert the block in address order so that colesce will be possible therefore achieve a lower external fragrance rate.
*/

static void *heap_listp; // Points to the start of the explict list
static void *heap_begin;
static void *heap_end;

static int count_malloc = 0;
static int count_free = 0;
static int count_realloc = 0;

static int mm_check(void)
{
    // first stage
    void *iter = heap_listp;
    assert(heap_listp != NULL);
    while (iter)
    {
        assert(GET_SIZE(HDRP(iter)) == GET_SIZE(FTRP(iter)));
        assert(GET_SIZE(HDRP(iter)) >= 0);
        assert(GET_ALLOC(FTRP(iter)) == 0);
        assert(GET_ALLOC(HDRP(iter)) == 0);
        iter = SUCC(iter);
    }
    return 1;
}

static void print_delimiter(char symbol, char *color)
{
    if (!DEBUG)
    {
        return;
    }
    char output[120];
    for (int i = 0; i < 120; ++i)
    {
        output[i] = symbol;
    }
    D_PRINT("%s%s%s\n", color, output, KNRM);
    return;
}

/*
* Traverse the explict linked-list and print the useful info about each node. This is used for debugging
* and should not be used for release version.
*/
static void heap_list_traversal(void)
{
    // mm_check();
    void *iter = heap_listp;
    if (iter == NULL)
    {
        D_PRINT("No free list at all!\n");
    }
    D_PRINT("%sheap_begin:%p, %sheap_listp:%p, %sheap_end:%p%s\n", KYEL, heap_begin, KBLU, heap_listp, KMAG, heap_end, KNRM);
    while (iter != NULL)
    {
        D_PRINT("ALLOC:%d,%d HDR:%p, %s[NODE:%p],%s FTR:%p, [SUCC:%p], [PRED:%p],size:%d\n", GET_ALLOC(HDRP(iter)), GET_ALLOC(FTRP(iter)), HDRP(iter), KGRN, iter, KNRM, FTRP(iter), SUCC(iter), PRED(iter), GET_SIZE(FTRP(iter)));
        iter = SUCC(iter);
    }
    //print_delimiter('-', KNRM);
    return;
}

/*
* This should be called after heap is extended when there would be two consecutive blocks in the linked-list which are also address space contigent
* Iterate through the linked-list to merge the elements.
*/
static void *merge_consecutive_element(void)
{
    void *iter = heap_listp;
    D_PRINT("Try to merge the neighboring blocks...\n");
    while (iter)
    {
        // D_PRINT("iter:%p\n", iter);
        if (!GET_ALLOC(FTRP(iter)) && !GET_ALLOC(FTRP(SUCC(iter))))
        {
            if (FTRP(iter) + WSIZE == HDRP(SUCC(iter)))
            {
                void *succ_succp = SUCC(SUCC(iter));
                size_t new_size = GET_SIZE(HDRP(iter)) + GET_SIZE(HDRP(SUCC(iter)));
                D_PRINT("Merge [%p],size:%d with [%p],size:%d\n", iter, GET_SIZE(HDRP(iter)), SUCC(iter), GET_SIZE(HDRP(SUCC(iter))));
                PUT(HDRP(iter), PACK(new_size, 0));
                PUT(FTRP(iter), PACK(new_size, 0));
                SUCC(iter) = succ_succp;
                PRED(succ_succp) = iter;
            }
        }
        iter = SUCC(iter);
    }
    return heap_listp;
}

/*
* When coalesce is failed, no block in the linked-list is found to be coalsced with, this function is to be called
* to insert the newly-freed block in the linked-list via the address space ordering to mitigate the external fragrancy.
*/
static int insert_empty_block(void *ptr)
{
    void *iter = heap_listp;
    void *lower_bound_ele = NULL;
    void *upper_bound_ele = NULL;

    while (iter)
    {
        if (iter < ptr)
        {
            lower_bound_ele = iter;
        }
        if (iter > ptr)
        {
            upper_bound_ele = iter;
            break;
        }
        iter = SUCC(iter);
    }
    SUCC(lower_bound_ele) = ptr;
    PRED(upper_bound_ele) = ptr;
    SUCC(ptr) = upper_bound_ele;
    PRED(ptr) = lower_bound_ele;
    return 1;
}

/*
* Whenever a new block is freed, we should check the neighbors(addresss-space contingent blocks)
* and try to coalesce them if the neighbors are not used.
*/
static int coalesce(void *ptr)
{
    address_len ajacent_prev_used = 1;
    address_len ajacent_next_used = 1;
    address_len size = GET_SIZE(HDRP(ptr));
    address_len prevp_size;

    void *ajacent_prevp;
    void *ajacent_nextp;

    void *ajacent_prev_footer = AJACENT_PREV_FOOTER(ptr);
    void *ajacent_next_header = AJACENT_NEXT_HEADER(ptr);
    prevp_size = GET_SIZE(ajacent_prev_footer);
    ajacent_prevp = ajacent_prev_footer - prevp_size + 2 * WSIZE;
    ajacent_nextp = ajacent_next_header + WSIZE;

    ajacent_prev_used = GET_ALLOC(ajacent_prev_footer);
    ajacent_next_used = GET_ALLOC(ajacent_next_header);

    if (ajacent_prev_used && ajacent_next_used)
    {
        D_PRINT("Nowhere to coalesce, try to insert...\n");
        return 0;
    }
    else if (!ajacent_prev_used && ajacent_next_used)
    {

        size += GET_SIZE(HDRP(ajacent_prevp));
        ptr = ajacent_prevp;
        D_PRINT("Coalesce with ajacent_prevp:%p, ajacent_prev_footer:%p alloc:%d,%d\n", ajacent_prevp, ajacent_prev_footer, GET_ALLOC(FTRP(ajacent_prevp)), GET_ALLOC(HDRP(ajacent_prevp)));
    }
    else if (ajacent_prev_used && !ajacent_next_used)
    {
        void *next_pred;
        void *next_succ;
        D_PRINT("Coalesce with %sajacent_nextp:[%p], ajacent_next_header:[%p]%s, size:%d alloc:%d,%d\n",
                KCYN, ajacent_nextp, ajacent_next_header, KNRM, GET_SIZE(HDRP(ajacent_nextp)),
                GET_ALLOC(FTRP(ajacent_nextp)), GET_ALLOC(HDRP(ajacent_nextp)));
        size += GET_SIZE(HDRP(ajacent_nextp));
        if ((next_pred = PRED(ajacent_nextp)) != NULL)
        {
            SUCC(next_pred) = ptr;
        }
        if ((next_succ = SUCC(ajacent_nextp)) != NULL)
        {
            PRED(next_succ) = ptr;
        }
        PRED(ptr) = next_pred;
        SUCC(ptr) = next_succ;
    }
    else if (!ajacent_prev_used && !ajacent_next_used)
    {
        void *next_succ;
        D_PRINT("Coalesce with %sajacent_nextp:[%p](ajacent_next_header:[%p]%s, size:%d alloc:%d,%d) and %sajacent_prevp:[%p], ajacent_prev_header:[%p]%s, size:%d alloc:%d,%d\n",
                KCYN, ajacent_nextp, ajacent_next_header, KNRM,
                GET_SIZE(HDRP(ajacent_nextp)), GET_ALLOC(FTRP(ajacent_nextp)), GET_ALLOC(HDRP(ajacent_nextp)),
                KCYN, ajacent_prevp, ajacent_prev_footer, KNRM,
                GET_SIZE(HDRP(ajacent_prevp)), GET_ALLOC(FTRP(ajacent_prevp)), GET_ALLOC(HDRP(ajacent_prevp)));

        size = size + GET_SIZE(HDRP(ajacent_prevp)) + GET_SIZE(HDRP(ajacent_nextp));
        if ((next_succ = SUCC(ajacent_nextp)) != NULL)
        {
            PRED(next_succ) = ajacent_prevp;
        }
        SUCC(ajacent_prevp) = next_succ;
        ptr = ajacent_prevp;
    }

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    return 1;
}

static void *extend_heap(size_t words)
{
    size_t size_without_epilogue;
    void *old_brk;
    void *new_empty_blkp;
    void *new_epilogue;
    void *old_epilogue_pred;

    D_PRINT("Extend heap by %ld\n", words);
    if ((old_brk = mem_sbrk(words)) == -1)
    {
        return NULL;
    }

    heap_end += words;
    new_epilogue = heap_end - 3 * WSIZE;
    new_empty_blkp = old_brk - 3 * WSIZE;
    old_epilogue_pred = PRED(new_empty_blkp);

    PUT(HDRP(new_epilogue), PACK(4 * WSIZE, 1));
    PUT(FTRP(new_epilogue), PACK(4 * WSIZE, 1));
    SUCC(new_epilogue) = NULL;
    PRED(new_epilogue) = new_empty_blkp;

    PUT(HDRP(new_empty_blkp), PACK(words, 0)); // now the epilogue is no longer the sentinel, so we can actually use that memory
    PUT(FTRP(new_empty_blkp), PACK(words, 0)); // now the epilogue is no longer the sentinel, so we can actually use that memory
    SUCC(new_empty_blkp) = new_epilogue;
    PRED(new_empty_blkp) = old_epilogue_pred;

    return merge_consecutive_element();
}

/*
* best-fit strategy
*/
void *find_fit(size_t size)
{
    void *ptr = heap_listp;
    size_t blk_size;
    size_t size_diff = UINT64_MAX;
    void *ptr_most_fit;
    int found_suitable_blk = 0;
    for (; ptr != NULL; ptr = SUCC(ptr))
    {
        blk_size = GET_SIZE(HDRP(ptr));
        if (!GET_ALLOC(HDRP(ptr)) && blk_size >= size)
        {
            size_t size_remain = blk_size - size;
            if (size_remain < size_diff)
            {
                size_diff = size_remain;
                ptr_most_fit = ptr;
                found_suitable_blk = 1;
            }
        }
    }
    if (found_suitable_blk)
    {
        D_PRINT("Find a block of size:%d, [HDRP:%p], [NODE:%p], [FTRP:%p], [PRED:%p], [SUCC:%p]\n", GET_SIZE(HDRP(ptr_most_fit)), HDRP(ptr_most_fit), ptr_most_fit, FTRP(ptr_most_fit), PRED(ptr_most_fit), SUCC(ptr_most_fit));
        return ptr_most_fit;
    }
    return NULL;
}

/*
* Cut a block in size of 8-bytes padding.
* Add the remaining free part to the linked-list if its size is larger than 4*WSIZE, which is the minimal block size.
*/
void place(void *ptr, size_t size_requested)
{
    size_t size_payload = GET_SIZE(HDRP(ptr));
    void *pred = PRED(ptr);
    void *succ = SUCC(ptr);
    // if the block size is larger than the size_requested by at least 4*WSIZE
    // we can split up the remain part and added to the linked-list
    if (size_payload > 4 * WSIZE + size_requested) // unsigned value!!!
    {
        void *remain_blk_p = ptr + size_requested;
        if (pred != NULL)
        {
            SUCC(pred) = remain_blk_p;
            D_PRINT("%sPRED[%p]%s change from [%p] to [%p]\n", KCYN, pred, KNRM, ptr, remain_blk_p);
        }
        if (succ != NULL)
        {
            PRED(succ) = remain_blk_p;
            D_PRINT("%sSUCC[%p]%s change from [%p] to [%p]\n", KCYN, succ, KNRM, ptr, remain_blk_p);
        }
        PUT(HDRP(ptr), PACK(size_requested, 1));
        PUT(FTRP(ptr), PACK(size_requested, 1));

        PUT(HDRP(remain_blk_p), PACK(size_payload - size_requested, 0));
        PUT(FTRP(remain_blk_p), PACK(size_payload - size_requested, 0));
        PRED(remain_blk_p) = pred;
        SUCC(remain_blk_p) = succ;
        D_PRINT("Split up, [ptr:%p] [HDRP:%p] [FTRP:%p]\n", remain_blk_p, HDRP(remain_blk_p), FTRP(remain_blk_p));
    }
    // use the whole block, update the predecessor and successor
    else
    {
        PUT(HDRP(ptr), PACK(size_payload, 1));
        PUT(FTRP(ptr), PACK(size_payload, 1));

        if (pred != NULL)
        {
            SUCC(pred) = succ;
        }
        if (succ != NULL)
        {
            PRED(succ) = pred;
        }
    }
    return;
}

/*
 * mm_init - initialize the malloc package.
 * prologue: size 4*WSIZE and allocated true
 * epilogue: size 4*WSIZE and allocated true
 * We use them as the sentinels so that when we try to coalesce the newly freed block by inspecting
 * the neighboring blocks(ajacent in the heap address space), it would be much easier for the corner
 * cases where the block is near the start or the end of the heap.
 */
int mm_init(void)
{
    if ((heap_begin = mem_sbrk(8 * WSIZE)) == (void *)-1)
    {
        return -1;
    }
    PUT(heap_begin, PACK(4 * WSIZE, 1));               // prologue header
    PUT(heap_begin + 3 * WSIZE, PACK(4 * WSIZE, 1));   // prologue footer
    PRED(heap_begin + WSIZE) = NULL;                   // prologue predecessor
    SUCC(heap_begin + WSIZE) = heap_begin + 5 * WSIZE; // prologue successor

    PUT(heap_begin + 4 * WSIZE, PACK(4 * WSIZE, 1));   // epilogue header
    PUT(heap_begin + 7 * WSIZE, PACK(4 * WSIZE, 1));   // epilogue footer
    PRED(heap_begin + 5 * WSIZE) = heap_begin + WSIZE; // epilogue predecessor
    SUCC(heap_begin + 5 * WSIZE) = NULL;               // epilogue successor

    heap_listp = heap_begin + WSIZE;
    heap_end = heap_begin + 8 * WSIZE;
    if (extend_heap(CHUNKSIZE) == NULL)
    {
        return -1;
    }
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{

    //print_delimiter('=', KRED);
    D_PRINT("%s[%d] %smalloc(%d)\n", KRED, count_malloc++, KNRM, size);
    D_PRINT("First check the linked-list:\n");
    //heap_list_traversal();

    size_t padded_size;
    size_t extended_size;
    void *ptr;
    if (size == 0)
    {
        return NULL;
    }
    padded_size = (size + ALIGN - 1) / ALIGN * ALIGN + 2 * WSIZE; // 2*WSIZE for header and footer
    if ((ptr = find_fit(padded_size)) != NULL)
    {
        place(ptr, padded_size);

        D_PRINT("Find spare memory in the linked-list, now the linked-list becomes:\n");
        //heap_list_traversal();
        return ptr;
    }
    extended_size = padded_size > CHUNKSIZE ? padded_size : CHUNKSIZE;
    D_PRINT("Can not find the block with requested size, extend heap by %d\n", extended_size);
    if ((ptr = extend_heap(extended_size)) == NULL)
    {
        return NULL;
    }

    D_PRINT("After extending, now the linked-list becomes:\n");
    //heap_list_traversal();

    ptr = find_fit(padded_size);
    place(ptr, padded_size);
    return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 * Warning: try to free a unallocated memory or memory already freed may cause unknown failure issues
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    //print_delimiter('=', KRED);
    D_PRINT("%s[%d] %sfree(%d)\n", KRED, count_free++, KNRM, size);
    D_PRINT("ALLOC:%d,%d HDR:%p, %s[NODE:%p],%s FTR:%p,size:%d\n", GET_ALLOC(HDRP(ptr)), GET_ALLOC(FTRP(ptr)), HDRP(ptr), KGRN, ptr, KNRM, FTRP(ptr), GET_SIZE(FTRP(ptr)));
    D_PRINT("Before free, the linked-list is:\n");
    //heap_list_traversal();

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    if (coalesce(ptr) == 0)
    {
        insert_empty_block(ptr);
    }

    D_PRINT("After free, the linked-list is:\n");
    //heap_list_traversal();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t size_allocted_before;
    size_t size_padded;
    //print_delimiter('=', KYEL);
    D_PRINT("%s[%d] %srealloc(%p, %d)\n", KRED, count_realloc++, KNRM, ptr, size);
    if (ptr == NULL)
    {
        D_PRINT("Arg ptr is NULL, realloac is euivalent to malloc.\n");
        return mm_malloc(size);
    }

    if (size == 0)
    {
        D_PRINT("Arg size is 0, realloac is euivalent to free.\n");
        mm_free(ptr);
        return;
    }
    D_PRINT("ALLOC:%d,%d HDR:%p, %s[NODE:%p],%s FTR:%p,size:%d,%d\n", GET_ALLOC(HDRP(ptr)), GET_ALLOC(FTRP(ptr)), HDRP(ptr), KGRN, ptr, KNRM, FTRP(ptr), GET_SIZE(HDRP(ptr)), GET_SIZE(FTRP(ptr)));
    size_padded = (size + ALIGN - 1) / ALIGN * ALIGN + 2 * WSIZE; // 2*WSIZE for header and footer
    size_allocted_before = GET_SIZE(FTRP(ptr));
    if (size_allocted_before >= size_padded)
    {
        D_PRINT("Requested size < size allocated before, now calling malloc\n");
        if (size_allocted_before >= 2 * size_padded - 2 * WSIZE + 4 * WSIZE)
        {
            void *remain_part_ptr = ptr + size_padded;
            size_t size_remain = size_allocted_before - size_padded;
            PUT(HDRP(ptr), PACK(size_padded, 1));
            PUT(FTRP(ptr), PACK(size_padded, 1));

            PUT(HDRP(remain_part_ptr), PACK(size_remain, 0));
            PUT(FTRP(remain_part_ptr), PACK(size_remain, 0));
            if (coalesce(remain_part_ptr) == 0)
            {
                insert_empty_block(remain_part_ptr);
            }

            D_PRINT("Ptr returned: ALLOC:%d,%d HDR:%p, %s[NODE:%p],%s FTR:%p,size:%d\n", GET_ALLOC(HDRP(ptr)), GET_ALLOC(FTRP(ptr)), HDRP(ptr), KGRN, ptr, KNRM, FTRP(ptr), GET_SIZE(FTRP(ptr)));
            D_PRINT("Ptr freed:    ALLOC:%d,%d HDR:%p, %s[NODE:%p],%s FTR:%p,size:%d\n", GET_ALLOC(HDRP(remain_part_ptr)), GET_ALLOC(FTRP(remain_part_ptr)), HDRP(remain_part_ptr), KGRN, remain_part_ptr, KNRM, FTRP(remain_part_ptr), GET_SIZE(FTRP(remain_part_ptr)));
            //print_delimiter('=', KYEL);
            //heap_list_traversal();
        }
        else
        {
            D_PRINT("Ptr returned: ALLOC:%d,%d HDR:%p, %s[NODE:%p],%s FTR:%p,size:%d\n", GET_ALLOC(HDRP(ptr)), GET_ALLOC(FTRP(ptr)), HDRP(ptr), KGRN, ptr, KNRM, FTRP(ptr), GET_SIZE(FTRP(ptr)));
            //heap_list_traversal();
        }
        return ptr;
    }
    else
    {
        void *ptr_newly_allocated;
        D_PRINT("Requested size > size allocated before, now calling malloc\n");
        size_padded = 2 * size_padded - 2 * WSIZE;
        ptr_newly_allocated = mm_malloc(size_padded);

        // only copy the data, without metadata!!!
        for (size_t i = 0; i < size_allocted_before - 2 * WSIZE; ++i)
        {
            *((uint8_t *)ptr_newly_allocated + i) = *((uint8_t *)ptr + i);
        }
        mm_free(ptr);
        D_PRINT("Ptr returned: ALLOC:%d,%d HDR:%p, %s[NODE:%p],%s FTR:%p,size:%d\n", GET_ALLOC(HDRP(ptr_newly_allocated)), GET_ALLOC(FTRP(ptr_newly_allocated)), HDRP(ptr_newly_allocated), KGRN, ptr_newly_allocated, KNRM, FTRP(ptr_newly_allocated), GET_SIZE(FTRP(ptr_newly_allocated)));
        D_PRINT("Ptr freed:    ALLOC:%d,%d HDR:%p, %s[NODE:%p],%s FTR:%p,size:%d\n", GET_ALLOC(HDRP(ptr)), GET_ALLOC(FTRP(ptr)), HDRP(ptr), KGRN, ptr, KNRM, FTRP(ptr), GET_SIZE(FTRP(ptr)));
        //print_delimiter('=', KYEL);
        //heap_list_traversal();

        return ptr_newly_allocated;
    }
    //print_delimiter('=', KYEL);
    return NULL;
}
