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
    char *tmp_host = ctx_lb_host(req->path, req->stmt);
    printf("proxy to host \x1b[38;5;50m%s\x1b[0m\n", tmp_host);
    free(tmp_host);
    int target_socket = net_connect("localhost", ctx_target_port());
    if (target_socket < 0)
    {
        perror("proxy connect");
        char message[] = "mintdb pool - Error connecting to target";
        char content_type[] = "text/plain; charset=utf-8";
        write_response(res, HTTP_INTERNAL_ERROR, content_type, message);
        shutdown(target_socket, SHUT_WR);
        close(target_socket);
        return 1;
    }
    if (send_bytes(target_socket, req->data) != 0)
    {
        printf("error reading target bytes\n");
        char message[] = "mintdb pool - Error sending data to target";
        char content_type[] = "text/plain; charset=utf-8";
        write_response(res, HTTP_INTERNAL_ERROR, content_type, message);
        shutdown(target_socket, SHUT_WR);
        close(target_socket);
        return 1;
    }

    if (read_bytes(target_socket, res) != 0)
    {
        printf("error reading target bytes\n");
        char message[] = "mintdb pool - Error reading target response";
        char content_type[] = "text/plain; charset=utf-8";
        write_response(res, HTTP_INTERNAL_ERROR, content_type, message);
        shutdown(target_socket, SHUT_WR);
        close(target_socket);
        return 1;
    }
    shutdown(target_socket, SHUT_WR);
    close(target_socket);

    return 0;
}
