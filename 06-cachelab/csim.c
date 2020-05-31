#include "cachelab.h"
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_LINE_LENGTH 100

typedef void *address;

typedef struct Count
{
    int hit;
    int miss;
    int eviction;
} Count;

typedef struct LRU
{
    void **entry;
    int *end;
} LRU;

typedef struct Cache
{
    void *table;
    int s;
    int E;
    int b;
} Cache;

static int cache_line_size;
static int cache_set_size;
static int cache_size;
static int verbose_flag;

/*

| t | s | b |

----------------------------

               *%*  Cache Layout  *%*

        |   Tag    |   Valid   | B0 | B1 | B2 | ... | Bn
*********************************************************
   | E0 |  8 byte    1 byte    1 byte
   ------------------------------------------------------
S0 |... |
   ------------------------------------------------------
   | En |
*********************************************************
   | E0 |
   ------------------------------------------------------
S1 |... |
   ------------------------------------------------------
   | En |
---------------------------------------------------------


     *%* LRU table *%*

   |   E0   |   E1   | ... |   En   |
   ----------------------------------
S0 | void *
   ----------------------------------
S1 |
   ----------------------------------
...|
   ----------------------------------
Sn |

*/

void addLruEntry(LRU *lru_table, int cache_set_idx, Cache *cache_ptr, void *idx_start, void **lru_entry_start)
{
    //TODO for missing
    int line_starting_idx = lru_table->end[cache_set_idx];
    lru_entry_start[lru_table->end[cache_set_idx]] = idx_start + line_starting_idx*cache_line_size;
    lru_table->end[cache_set_idx]++;
    return;
};

void replaceLruEntry(LRU * lru_table, int cache_set_idx, void **lru_entry_start){
    //TODO for eviction
    //Move the front element to back
    void* temp =lru_entry_start[0];
    for (int i = 0; i < lru_table->end[cache_set_idx] -1; ++i)
    {
        lru_entry_start[i] = lru_entry_start[i+1];
    }
    lru_entry_start[lru_table->end[cache_set_idx] - 1] = temp;
    return;

};

void updateLru(LRU *lru_table, int E, int cache_set_idx, void *cache_line_ptr, void **lru_entry_start)
{
    //TODO for hit
    for (int i = 0; i < lru_table->end[cache_set_idx]; ++i)
    {
        if (lru_entry_start[i] == cache_line_ptr)
        {
            void *temp = lru_entry_start[i];
            for (int j = i; j < lru_table->end[cache_set_idx] - 1; ++j)
            {
                lru_entry_start[j] = lru_entry_start[j + 1];
            }
            lru_entry_start[lru_table->end[cache_set_idx] - 1] = temp;
            return;
        }
        return;
    }
};

void accessCache(LRU *lru_table, Cache *cache_ptr, Count *count_ptr, char instruct, long addr, int size)
{
    unsigned cache_set_idx_mask = ((1 << (cache_ptr->s)) - 1);
    int cache_set_idx = (addr >> (cache_ptr->b)) & cache_set_idx_mask;
    long tag = addr & ~((1 << (cache_ptr->s + cache_ptr->b)) - 1); // lower b+s bits zero
    int tag_byte_len = 8;
    void *idx_start = cache_ptr->table + cache_set_idx * cache_set_size;
    int step = (instruct == 'M' ? 2 : 1);
    void **lru_entry_start = lru_table->entry + cache_set_idx * cache_ptr->E;

    if(verbose_flag){
        printf("%c %lx,%d ", instruct, addr, size);
    }

    for (int k = 0; k < step; ++k)
    {
        int find_cache = 0;
        void *cache_line_ptr = idx_start;
        for (int i = 0; i < cache_ptr->E; ++i)
        {
            if (*(long *)(cache_line_ptr) == tag)
            {
                if (*(char *)(cache_line_ptr + tag_byte_len) == 1)
                {
                    if (lru_table->end[cache_set_idx] == 0)
                    {
                        lru_entry_start[0] = cache_line_ptr;
                        lru_table->end[cache_set_idx]++;
                    }
                    updateLru(lru_table, cache_ptr->E, cache_set_idx, cache_line_ptr, lru_entry_start);

                    // TODO: cache hit, now check the size
                    count_ptr->hit++;
                    find_cache = 1;
                    if(verbose_flag){
                         printf("hit ");
                    }
                    // printf(" \tcache_set_idx: %d \ttag: 0x%lx \tblock_offset: 0x%x", cache_set_idx, tag, block_offset);
                    if (instruct != 'M')
                    {
                        if(verbose_flag){
                             printf("\n ");
                        }
                        return;
                    }
                }
            }
            if (i == cache_ptr->E - 1 && !find_cache)
            {
                int find_empty = 0;
                char *valid_ptr = NULL;
                count_ptr->miss++;
                if(verbose_flag){
                     printf("miss ");
                }
                for (int i = 0; i < cache_ptr->E; ++i)
                {
                    valid_ptr = (char *)(idx_start + cache_line_size * i + tag_byte_len);
                    if (*valid_ptr != 1)
                    {
                        *valid_ptr = 1;
                        find_empty = 1;
                        *(long *)(idx_start + cache_line_size * i) = tag; // change to new tag
                        addLruEntry(lru_table, cache_set_idx, cache_ptr, idx_start, lru_entry_start);
                        break;
                    }
                }
                if (!find_empty)
                {
                    valid_ptr = (char *)(lru_entry_start[0] + tag_byte_len);
                    *(long *)(lru_entry_start[0]) = tag; // change to new tag
                    count_ptr->eviction++;
                    *valid_ptr = 1;
                    replaceLruEntry(lru_table, cache_set_idx, lru_entry_start);
                }
            }
            cache_line_ptr = cache_line_ptr + cache_line_size;
        }
    }
    // printf(" \tcache_set_idx: %d \ttag: 0x%lx \tblock_offset: 0x%x\n", cache_set_idx, tag, block_offset);
    if(verbose_flag){
         printf("\n ");
    }
    return;
};

int main(int argc, char *argv[])
{
    int opt;
    int s, E, b;

    int tag_byte_len = 8;
    char trace_line_buf[MAX_LINE_LENGTH];
    char instruction;

    FILE *trace_file;
    LRU lru;
    Count count = {0, 0, 0};

    long accessed_addr = 0;
    int accessed_size = 0;

    while ((opt = getopt(argc, argv, "hv::s:E:b:t:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\t\n"
                   "Options\t\n"
                   "-h         Print this help message.\t\n"
                   "-v         Optional verbose flag.\t\n"
                   "-s <num>   Number of set index bits.\t\n"
                   "-E <num>   Number of lines per set.\t\n"
                   "-b <num>   Number of block offset bits.\t\n"
                   "-t <file>  Trace file.\t\n"
                   "\nExamples:\t\n"
                   "linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\t\n"
                   "linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\t\n");
            return 0;
        case 'v':
            verbose_flag = 1;
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            //TODO: add trace file
            trace_file = fopen(optarg, "r");
            break;
        default:
            return 0;
        }
    }

    cache_line_size = ((1 << b) + tag_byte_len + 1);
    cache_set_size = cache_line_size * E;
    cache_size = (1 << s) * cache_set_size;

    void *table = (void *)calloc(cache_size, 1);
    lru.end = (int *)calloc((1 << s) * E, sizeof(int));
    lru.entry = (void **)calloc((1 << s) * E, sizeof(void *));

    while (fgets(trace_line_buf, MAX_LINE_LENGTH, trace_file) != NULL)
    {
        if (trace_line_buf[0] == 'I')
        {
            continue;
        }
        int i = 0;
        while (trace_line_buf[i] == ' ')
        {
            ++i;
        }
        instruction = trace_line_buf[i++];
        while (trace_line_buf[i] == ' ')
        {
            ++i;
        }
        while (trace_line_buf[i] != ',')
        {
            if (trace_line_buf[i] >= '0' && trace_line_buf[i] <= '9')
            {
                accessed_addr = accessed_addr * 16 + (int)(trace_line_buf[i] - '0');
            }
            else if (trace_line_buf[i] >= 'a' && trace_line_buf[i] <= 'f')
            {
                accessed_addr = accessed_addr * 16 + (int)(trace_line_buf[i] - 'a' + 10);
            }
            ++i;
        }
        ++i;
        while (trace_line_buf[i] != '\n')
        {
            if (trace_line_buf[i] == '\0')
            {
                break;
            }
            accessed_size = 10 * accessed_size + ((int)trace_line_buf[i] - 48);
            ++i;
        }

        Cache cache = {table, s, E, b};
        accessCache(&lru, &cache, &count, instruction, accessed_addr, accessed_size);
        accessed_addr = 0;
        accessed_size = 0;
    }
    printSummary(count.hit, count.miss, count.eviction);
    free(table);
    return 0;
}
