#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>

#include "server.h"
#include "router.h"

int server_socket;

int read_request(int client_socket, Request *request)
{
    read_bytes(client_socket, &request->data);
    if (extract_target(request, request->data) != 0 || parse_body(request, request->data) != 0)
    {
        return 1;
    }
    return 0;
}
void cleanup(int socket, Request *request, char *data, char *response)
{
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
    if (read_request(client_socket, request) != 0)
    {
        printf("error reading request\n");
        char message[] = "mintdb pool - Error reading request";
        send_internal_error(client_socket, message);
        cleanup(client_socket, request, request->data, response);
        return;
    }
    printf("@\x1b[38;5;50m%s \x1b[0m%s\n", request->method, request->path);

    router(request, &response);

    send_bytes(client_socket, response);

    cleanup(client_socket, request, request->data, response);
    printf("closing connection\n");
    return;
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
int server(int port)
{
    ctx_init();
    graceful_shutdown();
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("error initializing socket");
        return 1;
    }
    int srv_flag = 1;
    setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, &srv_flag, sizeof(int));
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_ANY)};

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error binding to addr");
        return 1;
    }
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("error listening");
        return 1;
    }
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
        setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
        int *client_socket_ptr = (int *)malloc(sizeof(int));

        *client_socket_ptr = client_socket;
        ctx_add_task(handle_client, client_socket_ptr);
    }
    close(server_socket);
    ctx_cleanup();
    return 0;
}