#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define MAX_NUM 128
#define MAX_BUF_LEN 1024

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int parse_http_request(char *request)
{
}

void send_http_request(const char *request)
{
}

void receive_content(char *buffer, int max_byte_recv)
{
}

void send_client_content(char *buffer, int max_byte_send)
{
}

int main(int argc, char *argv[])
{
    int listen_fd, conn_fd;
    rio_t rio_buf;
    char host[MAX_NUM];
    char port[MAX_NUM];
    char buf_recv[MAX_BUF_LEN];
    // char buf_send[MAX_NUM];
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
    conn_fd = Accept(listen_fd, &sock_storage, &sock_len);
    Getnameinfo(&sock_storage, sock_len, host, MAX_NUM, port, MAX_NUM, 0);
    printf("Proxy accepted the connection by the client<%s, %s>...\n", host, port);
    Rio_readinitb(&rio_buf, conn_fd);
    while (1)
    {
        Rio_readlineb(&rio_buf, buf_recv, MAX_BUF_LEN);
        printf("Proxy receive %ld byte(s) from client, contents: %s", strlen(buf_recv), buf_recv);

        //TODO:
        parse_http_request();
        send_http_request();
        receive_content();
        send_client_content();

        Rio_writen(conn_fd, buf_recv, strlen(buf_recv));
    }

    return 0;
}
