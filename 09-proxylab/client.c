#include "csapp.h"

#define MAX_NUM 128

int main(int argc, char *argv[])
{
    int client_fd;
    char *host;
    char *port;
    char buffer_recv[MAX_NUM];
    char buffer_send[MAX_NUM];
    rio_t rio_buf;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <host, port>\n", argv[0]);
        exit(-1);
    }
    host = argv[1];
    port = argv[2];
    client_fd = Open_clientfd(host, port);
    printf("Client connected to proxy<%s, %s>...\n", host, port);
    Rio_readinitb(&rio_buf, client_fd);
    while (Fgets(buffer_send, MAX_NUM, stdin) != NULL)
    {
        Rio_writen(client_fd, buffer_send, strlen(buffer_send));
        Rio_readlineb(&rio_buf, buffer_recv, MAX_NUM);
        printf("%s\n", buffer_recv);
    }
    return 0;
}