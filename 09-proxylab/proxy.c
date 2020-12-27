#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define MAX_NUM 128
#define MAX_BUF_LEN 512

struct HttpRequest
{
    char *request;
    char *host;
    char *file;
};

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int parse_http_request(char *input, char *output, char *request_host)
{
    int i = 0;
    int count = 0;
    char buffer[3][64];
    char *request_type = buffer[0];
    char request_loc[64];
    char *http_version;
    char *token;
    char *host_start;

    token = strtok(input, " ");
    while (token != NULL)
    {
        memcpy(buffer[count++], token, strlen(token) + 1);
        token = strtok(NULL, " ");
    }
    // Only support HTTP GET method!
    if (strcmp(request_type, "GET") != 0)
    {
        return -1;
    }

    // char *host_start = strip_host_prefix(buffer);
    if (strstr(buffer[1], "https://") != NULL)
    {
        host_start = &buffer[1][8];
    }
    else if (strstr(buffer[1], "http://") != NULL)
    {
        host_start = &buffer[1][7];
    }
    else
    {
        host_start = &buffer[1][0];
    }
    while (host_start[i] != '/')
    {
        i++;
    }

    memcpy(request_host, host_start, i);
    memcpy(request_host + i, "\0", 1);
    memcpy(request_loc, host_start + i, strlen(buffer[1]) - strlen(request_host) + 1);

    sprintf(output, "%s %s HTTP/1.0\r\n", request_type, request_loc);
    sprintf(output, "%sHost: %s\r\n", output, request_host);
    sprintf(output, "%s%s", output, user_agent_hdr);
    sprintf(output, "%sConnection: close\r\nProxy-Connection: close\r\n\r\n", output); // Holy fuck!!!  \r\n\r\n

    printf("request_type: %s\n", request_type);
    printf("request_host: %s\n", request_host);
    printf("request_loc: %s\n", request_loc);

    return 0;
}

void do_proxy(const char *http_request, const char *request_host, int conn_fd)
{
    rio_t rio_server_buf;
    char recv_buffer[MAX_BUF_LEN];
    int client_fd;
    char hostname[32];
    char port[16];
    int i = 0;
    int hostname_len = 0;
    int port_len = 0;
    memset(hostname, 0, sizeof(hostname));
    memset(port, 0, sizeof(port));
    char *p = request_host;
    while (request_host[i] != ':')
    {
        hostname_len++;
        i++;
    }
    memcpy(hostname, request_host, hostname_len);

    while (request_host[i] != 0)
    {
        port_len++;
        i++;
    }
    memcpy(port, request_host + hostname_len + 1, port_len);

    client_fd = Open_clientfd(hostname, port);
    printf("Connected to server!\n");
    Rio_readinitb(&rio_server_buf, client_fd);
    Rio_writen(client_fd, http_request, strlen(http_request)); // Send request to server
    Rio_writen(client_fd, "Connection: close\n", strlen("Connection: close\n"));
    Rio_writen(client_fd, "Proxy-Connection: close\n", strlen("Proxy-Connection: close\n"));

    int byte_read = 0;
    while ((byte_read = Rio_readnb(&rio_server_buf, recv_buffer, MAX_BUF_LEN)) != 0) // Get reply from server
    {
        printf("%s", recv_buffer);
        Rio_writen(conn_fd, recv_buffer, byte_read); // Forward reply to client
    }
}

void *proxy_thread(void *conn_fd_p)
{
    char http_request[MAX_BUF_LEN];
    char buf_recv[MAX_BUF_LEN];
    rio_t rio_client_buf;
    int conn_fd = *(int *)conn_fd_p;
    free(conn_fd_p);

    Rio_readinitb(&rio_client_buf, conn_fd);
    while (1)
    {
        char request_host[64];
        Rio_readlineb(&rio_client_buf, buf_recv, MAX_BUF_LEN);
        printf("Proxy receive %ld byte(s) from client, contents: %s", strlen(buf_recv), buf_recv);
        // Rio_writen(conn_fd, buf_recv, strlen(buf_recv));

        memset(http_request, 0, MAX_BUF_LEN);
        if (parse_http_request(buf_recv, http_request, request_host) != 0)
        {
            // TODO too many errors would cause client close the connection
            // char *error_msg = "HTTP requested type error, only GET method supported!\n";
            // printf("%s", error_msg);
            // Rio_writen(conn_fd, error_msg, strlen(error_msg));
            continue;
        }
        else
        {
            do_proxy(http_request, request_host, conn_fd);
        }
    }
    return;
}

int main(int argc, char *argv[])
{
    int listen_fd;

    char host[MAX_NUM];
    char port[MAX_NUM];
    char buf_send[MAX_BUF_LEN];

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

    while (1)
    {
        int *conn_fd_p = malloc(sizeof(int));
        pthread_t *tid_p = malloc(sizeof(pthread_t));
        *conn_fd_p = Accept(listen_fd, &sock_storage, &sock_len);
        Getnameinfo(&sock_storage, sock_len, host, MAX_NUM, port, MAX_NUM, 0);
        printf("Proxy accepted the connection by the client<%s, %s>...\n", host, port);
        Pthread_create(tid_p, NULL, proxy_thread, (void *)conn_fd_p);
        Pthread_detach(*tid_p);
        free(tid_p);
        printf("\033[33m==========================================================\033[0m\n");
    }

    return 0;
}
