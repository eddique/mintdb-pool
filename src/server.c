#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>

#include "server.h"
#include "router.h"

int server_socket;

void cleanup(int socket, Request *request, char *data, char *response)
{
    shutdown(socket, SHUT_WR);
    close(socket);
    if (data != NULL)
    {
        free(data);
    }
    if (request != NULL)
    {
        free(request);
    }
    if (response != NULL)
    {
        free(response);
    }
}
void handle_signal(int signal, siginfo_t *siginfo, void *context)
{
    if (signal == SIGINT)
    {
        printf("\ninterrupt signal received\n");
    }
    else if (signal == SIGTERM)
    {
        printf("\ntermination signal received\n");
    }
    else
    {
        printf("\nunknown signal received\n");
    }
    printf("\x1b[38;5;50mshutting down gracefully\x1b[0m\n");
    ctx_cleanup();
    shutdown(server_socket, SHUT_WR);
    close(server_socket);
    exit(0);
}
void graceful_shutdown()
{
    struct sigaction action;
    action.sa_sigaction = handle_signal;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) < 0)
    {
        perror("sigaction SIGINT");
        return;
    }
    if (sigaction(SIGTERM, &action, NULL) < 0)
    {
        perror("sigaction SIGTERM");
        return;
    }
}
void handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg);
    char *response = NULL;
    Request *request = (Request *)malloc(sizeof(Request));
    if (request == NULL)
    {
        perror("malloc");
        char message[] = "mintdb pool - Internal Error";
        send_internal_error(client_socket, message);
        cleanup(client_socket, request, request->data, response);
        return;
    }
    request->data = NULL;
    if (read_request(client_socket, request) != 0 || request->data == NULL)
    {
        printf("error reading request\n");
        char message[] = "mintdb pool - Error reading request";
        send_internal_error(client_socket, message);
        cleanup(client_socket, request, request->data, response);
        return;
    }
    printf("@\x1b[38;5;50m%s \x1b[0m%s \x1b[38;5;50m%s\x1b[0m\n",
           request->method, request->path, request->stmt);

    router(request, &response);

    send_bytes(client_socket, response);

    cleanup(client_socket, request, request->data, response);
    printf("closing connection\n");
    return;
}
int server()
{
    ctx_init();
    graceful_shutdown();
    int port = ctx_port();
    server_socket = net_init_server(port);
    printf("\x1b[38;5;50mlistening on http://localhost:%d\x1b[0m\n", port);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        printf("waiting for connection\n");
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1)
        {
            perror("failed to accept connection");
            continue;
        }

        int flag = 1;
        setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
        setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
        int *client_socket_ptr = (int *)malloc(sizeof(int));

        *client_socket_ptr = client_socket;
        ctx_add_task(handle_client, client_socket_ptr);
    }
    shutdown(server_socket, SHUT_WR);
    close(server_socket);
    ctx_cleanup();
    return 0;
}