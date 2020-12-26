#include "csapp.h"

#define MAX_NUM 1024

int main(int argc, char *argv[])
{
    int client_fd;
    int num_recv;
    char *host;
    char *port;
    char buffer_recv[MAX_NUM];
    char buffer_send[MAX_NUM];
    rio_t rio_buf;
    rio_t buf_stdin;

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
    Rio_readinitb(&buf_stdin, STDIN_FILENO);

    while (Fgets(buffer_send, MAX_NUM, stdin) != NULL)
    {
        Rio_writen(client_fd, buffer_send, strlen(buffer_send));
        //TODO what about time delay?
        while ((num_recv = Rio_readnb(&rio_buf, buffer_recv, MAX_NUM)) > 0)
        {
            printf("------------------------>  num_recv:%d\n", num_recv);
            /*
            WARNING: if not we will always print the content of buffer_send every loop!
            By using %s we assume '\0' in the end, buffer_recv and buffer_send are contigent in memory!
            */
            // Fputs(buffer_recv, stdout);
            Rio_writen(STDIN_FILENO, buffer_recv, num_recv);
            // printf("%s", buffer_recv);
        }
        printf("------------------------------------------------\n");
    }
    return 0;
}