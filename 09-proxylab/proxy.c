#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define HOST_LEN 64
#define FILE_LEN 64
#define MAX_NUM 128
#define MAX_BUF_LEN 512
#define MAX_LINE_LEN 64

#define KRST "\033[0m"
#define KRED "\033[31m"
#define KGRN "\033[32m"
#define KYEL "\033[33m"
#define KBLU "\033[34m"
#define KPUR "\033[35m"

#define K_METHOD 1
#define K_HOST 2
#define K_FILE 3
#define K_UNKNOWN 4

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

struct MemCache
{
    char host[HOST_LEN];
    char file[FILE_LEN];
    char *data;
    int size;
    struct MemCache *next;
};

struct MemCacheManager
{
    struct MemCache *head;
    int cached_size;
    int cached_num;
};

struct MemCacheManager cache_manager;

//TODO Add synchronization and lock for multi_thread
struct MemCache *check_cached(char *request_host, char *request_file)
{
    struct MemCache *cur = cache_manager.head;
    struct MemCache *prev = NULL;
    while (cur)
    {
        if ((strcmp((char *)cur->host, request_host) == 0) && (strcmp((char *)cur->file, request_file) == 0))
        {
            if (prev != NULL)
            {
                prev->next = cur->next;
            }
            // change cur as head as it's recently visited
            cur->next = cache_manager.head;
            cache_manager.head = cur;
            return cur;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

//TODO Add synchronization and lock for multi_thread
int cache_oject(char *request_host, char *request_file, char *recv_buf, int object_size)
{
    struct MemCache *new_head = (struct MemCache *)malloc(sizeof(struct MemCache));
    new_head->data = (char *)malloc(object_size);
    memcpy((void *)new_head->data, (void *)recv_buf, object_size);
    memcpy((void *)new_head->host, (void *)request_host, HOST_LEN);
    memcpy((void *)new_head->file, (void *)request_file, FILE_LEN);

    new_head->size = object_size;
    new_head->next = cache_manager.head;
    cache_manager.head = new_head;
    if (cache_manager.cached_size + object_size > MAX_CACHE_SIZE)
    {
        struct MemCache *cur = cache_manager.head;
        struct MemCache *prev = NULL;
        while (cur)
        {
            if (cur->next == NULL)
            {
                break;
            }
            prev = cur;
            cur = cur->next;
        }
        // clean the LRU object
        free((void *)cur->data);
        free((void *)cur);
        prev->next = NULL;
    }
    return 0;
}

/**
 * @brief parse the raw http request and extract the request host and request file, generate the http request
 * which would be sent to server if necessary
 * @param[in]  input                  input http request string
 * @param[out] request_literal        output http request sent to server
 * @param[out] request_host           hostname: [ip:port]
 * @param[out] request_file           requeste file path on server
 * @return int return 0 on success, return -1 on failure
 */
int parse_http_request(char *input, char *request_literal, char *request_host, char *request_file)
{
    int i = 0;
    int count = 0;
    char buffer[3][64];
    char *http_version;
    char *host_start;

    char *token;
    char buf_line[MAX_LINE_LEN];
    int first_line = 1;
    char request_type[MAX_LINE_LEN];
    memset(request_type, 0, MAX_LINE_LEN);
    char *request_type_ptr = request_type;
    char *request_host_ptr = request_host;
    char *request_file_ptr = request_file;

    printf("%s[proxy server] client full http request:%s\n-----------------------------------\n%s-----------------------------------\n", KGRN, KRST, input);
    token = strtok(input, "\r\n");
    while (token != NULL)
    {
        char *buf_ptr = buf_line;
        memset(buf_line, 0, MAX_LINE_LEN);
        memcpy(buf_line, token, strlen(token) + 1);
        token = strtok(NULL, "\r\n");
        // tackle the start line of http request
        if (first_line)
        {
            int next_token_type = K_METHOD;
            for (int j = 0; j < MAX_BUF_LEN; ++j)
            {
                switch (next_token_type)
                {
                case K_METHOD:
                    if (*buf_ptr == ' ')
                    {
                        next_token_type = K_HOST;
                    }
                    *request_type_ptr++ = *buf_ptr++;
                    break;
                case K_HOST:
                    if (*buf_ptr == '/')
                    {
                        if (*(buf_ptr - 1) != '/' && *(buf_ptr - 1) != ':')
                        {
                            next_token_type = K_FILE;
                            // do not advance buf_ptr as we need it for K_FILE
                            break;
                        }
                    }
                    *request_host_ptr++ = *buf_ptr++;
                    break;
                case K_FILE:
                    if (*buf_ptr == ' ')
                    {
                        next_token_type = K_UNKNOWN;
                        break;
                    }
                    *request_file_ptr++ = *buf_ptr++;
                    break;
                case K_UNKNOWN:
                    break;
                }
            }
            printf("%s[proxy server] client request_type: %s%s\n", KGRN, request_type, KRST);
            printf("%s[proxy server] client request_host: %s%s\n", KGRN, request_host, KRST);
            printf("%s[proxy server] client request_file: %s%s\n", KGRN, request_file, KRST);
            if (strcmp(request_type, "GET ") != 0)
            {
                printf("%s[proxy server] request method:%s not supported! %s\n", KGRN, request_type, KRST);
                return -1;
            }
            first_line = 0;
            sprintf(request_literal, "%s%s HTTP/1.0\r\n", request_type, request_file);
            sprintf(request_literal, "%sHost: %s\r\n", request_literal, request_host);
            sprintf(request_literal, "%s%s", request_literal, user_agent_hdr);
            sprintf(request_literal, "%sConnection: close\r\nProxy-Connection: close\r\n\r\n", request_literal);
        }
        // tackle header and body of http request
        else
        {
            // do nothing
        }
    }
    printf("%s[proxy server] forwared request: %s\n%s\n", KGRN, KRST, request_literal);
    return 0;
}

void do_proxy(const char *http_request, const char *request_host, char *request_file, int conn_fd)
{
    // A naive implementation, recv_buffer can be a finer granularity
    char recv_buffer[MAX_OBJECT_SIZE];
    rio_t rio_server_buf;
    int client_fd;
    char hostname[HOST_LEN];
    char port[HOST_LEN];

    int host_addr_start = 0;
    if (strstr(request_host, "http://") != NULL)
    {
        host_addr_start = 7;
    }
    else if (strstr(request_host, "https://") != NULL)
    {
        host_addr_start = 8;
    }
    char request_host_in[HOST_LEN];
    memcpy(request_host_in, request_host + host_addr_start, HOST_LEN - host_addr_start);
    int len = strlen(request_host_in);
    int delimit_idx = len - 1;
    while (delimit_idx > 0)
    {
        if (request_host_in[delimit_idx] == ':')
        {
            break;
        }
        --delimit_idx;
    }

    memcpy(hostname, request_host_in, delimit_idx);
    memcpy(port, request_host_in + delimit_idx + 1, len - delimit_idx);

    client_fd = Open_clientfd(hostname, port);
    printf("Connected to server!\n");
    Rio_readinitb(&rio_server_buf, client_fd);
    Rio_writen(client_fd, http_request, strlen(http_request)); // Send request to server

    int byte_read = 0;
    int object_size = 0;
    while ((byte_read = Rio_readnb(&rio_server_buf, recv_buffer, MAX_OBJECT_SIZE)) != 0) // Get reply from server
    {
        printf("%s", recv_buffer);
        object_size += byte_read;
        Rio_writen(conn_fd, recv_buffer, byte_read); // Forward reply to client
    }
    if (object_size < MAX_OBJECT_SIZE)
    {
        cache_oject(request_host, request_file, recv_buffer, object_size);
    }
}

/**
 * @brief  main wroker thread to handle each socket connection
 *
 * @param conn_fd_p    socket connection fd
 * @return void*
 */
void *proxy_thread(void *conn_fd_p)
{
    char buf_line[MAX_LINE_LEN];
    char http_request[MAX_BUF_LEN];
    char buf_recv[MAX_BUF_LEN];
    char *buf_ptr = buf_recv;
    rio_t rio_client_buf;
    int conn_fd = *(int *)conn_fd_p;
    free(conn_fd_p);

    Rio_readinitb(&rio_client_buf, conn_fd);
    while (1)
    {
        char request_host[HOST_LEN];
        char request_file[FILE_LEN];
        memset(request_host, 0, HOST_LEN);
        memset(request_file, 0, FILE_LEN);
        memset(buf_recv, 0, MAX_BUF_LEN);
        memset(http_request, 0, MAX_BUF_LEN);

        int byte_read = 0;
        int64_t count = 0;
        // fetch and accumulate one http request
        while (1)
        {
            memset(buf_line, 0, MAX_LINE_LEN);
            if ((byte_read = Rio_readlineb(&rio_client_buf, buf_line, MAX_BUF_LEN)) < 0)
            {
                printf("%s[proxy server] readlineb error! %s\n", KGRN, KRST);
                // only way to exit
                return;
            }
            if (byte_read == 0)
            {
                count++;
                if (count % 10000000 == 0)
                {
                    printf("%s[proxy server] waiting... %s\n", KGRN, KRST);
                }
                continue;
            }
            printf("%s[proxy server] receives %ld byte(s) raw input: %s%s\n", KGRN, strlen(buf_line), KRST, buf_line);
            memcpy(buf_ptr, buf_line, byte_read);
            buf_ptr += byte_read;
            // An empty line indicates the end of one http request
            if (byte_read == 2 && strcmp(buf_line, "\r\n") == 0)
            {
                break;
            }
        }

        // parse the http request, only GET method is supported
        if (parse_http_request(buf_recv, http_request, request_host, request_file) != 0)
        {
            char *error_message = "HTTP/1.0 501 Not implemented\r\n\r\n";
            Rio_writen(conn_fd, error_message, strlen(error_message));
            continue;
        }

        // check if it's already cached. request to server if not
        struct MemCache *object = check_cached(request_host, request_file);
        if (object)
        {
            // forward reply to client
            Rio_writen(conn_fd, object->data, object->size);
        }
        else
        {
            do_proxy(http_request, request_host, request_file, conn_fd);
        }
    }
    // Should never reach here
    return;
}

int main(int argc, char *argv[])
{
    int listen_fd;

    char host[MAX_NUM];
    char port[MAX_NUM];

    char *listen_port;
    struct sockaddr_storage sock_storage;
    socklen_t sock_len;
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(-1);
    }
    listen_port = argv[1];
    listen_fd = Open_listenfd(listen_port);
    sock_len = sizeof(sock_storage);
    printf("%s[proxy server] main thread started, waiting for connection... %s\n", KGRN, KRST);
    while (1)
    {
        int *conn_fd_p = malloc(sizeof(int));
        pthread_t *tid_p = malloc(sizeof(pthread_t));
        *conn_fd_p = Accept(listen_fd, &sock_storage, &sock_len);
        Getnameinfo(&sock_storage, sock_len, host, MAX_NUM, port, MAX_NUM, 0);
        printf("%s[proxy server] accepted the connection by the client<%s, %s>...%s\n", KGRN, host, port, KRST);
        Pthread_create(tid_p, NULL, proxy_thread, (void *)conn_fd_p);
        Pthread_detach(*tid_p);
        free(tid_p);
    }
    struct MemCache *cur = cache_manager.head;
    while (cur)
    {
        struct MemCache *to_free = cur;
        cur = cur->next;
        free((void *)to_free->data);
        free((void *)to_free);
    }
    return 0;
}
