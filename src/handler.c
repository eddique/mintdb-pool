#include "handler.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int health_handler(Request *req, char **res)
{
    char message[] = "{\"ok\":true,\"message\":\"mintdb pool - all good!\"}";
    char content_type[] = "application/json; charset=utf-8";
    write_response(res, HTTP_OK, content_type, message);
    return 0;
}
int proxy_handler(Request *req, char **res)
{
    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(3000);
    struct hostent *host;

    char *tmp_host = ctx_lb_host(req->path, req->stmt);
    printf("sending to host: %s\n", tmp_host);
    free(tmp_host);
    host = gethostbyname("localhost");
    if (host == NULL)
    {
        perror("gethostbyname");
        char message[] = "mintdb pool - Error getting target host name";
        char content_type[] = "text/plain; charset=utf-8";
        write_response(res, HTTP_INTERNAL_ERROR, content_type, message);
        return -1;
    }
    memcpy(&target_addr.sin_addr, host->h_addr_list[0], host->h_length);
    int target_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (target_socket < 0)
    {
        printf("error creating target socket\n");
        char message[] = "mintdb pool - Error creating target socket";
        char content_type[] = "text/plain; charset=utf-8";
        write_response(res, HTTP_INTERNAL_ERROR, content_type, message);
        return -1;
    }
    int flag = 1;
    setsockopt(target_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
    if (connect(target_socket, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
    {
        perror("proxy connect");
        close(target_socket);
        char message[] = "mintdb pool - Error connecting to target";
        char content_type[] = "text/plain; charset=utf-8";
        write_response(res, HTTP_INTERNAL_ERROR, content_type, message);
        return 1;
    }
    size_t total_sent = 0;
    size_t request_length = strlen(req->data);
    while (total_sent < request_length)
    {
        ssize_t sent = send(target_socket, req->data + total_sent, request_length - total_sent, 0);
        if (sent == -1)
        {
            break;
        }
        total_sent += sent;
    }
    if (read_bytes(target_socket, res) != 0)
    {
        close(target_socket);
        printf("error reading target bytes\n");
        char message[] = "mintdb pool - Error reading target response";
        char content_type[] = "text/plain; charset=utf-8";
        write_response(res, HTTP_INTERNAL_ERROR, content_type, message);
        // send_internal_error(client_socket, message);
        // free(*response);
        // *response = NULL;
        return 1;
    }
    close(target_socket);

    return 0;
}
