#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#include <stdbool.h>
#include "server.h"
#include "handler.h"
typedef struct
{
    char method[MAX_CHARACTERS];
    char path[MAX_CHARACTERS];
    char stmt[MAX_CHARACTERS];
} Request;

int send_bytes(int socket, char *response)
{
    size_t total_sent = 0;
    size_t response_length = strlen(response);
    while (total_sent < response_length)
    {
        ssize_t sent = send(socket, response + total_sent, response_length - total_sent, 0);
        if (sent == -1)
        {
            break;
        }
        total_sent += sent;
    }
    return 0;
}
int send_internal_error(int socket, char *error)
{
    char *res = NULL;
    char content_type[] = "text/plain; charset=UTF-8";
    write_response(&res, HTTP_INTERNAL_ERROR, content_type, error);
    send_bytes(socket, res);
    free(res);
    return 0;
}
int parse_body(Request *req, char *data)
{
    char *stmt_key = strstr(data, "\"stmt\":");
    if (stmt_key != NULL)
    {
        stmt_key += 7;
        while (*stmt_key == ' ')
            stmt_key++;
        if (*stmt_key == '\"')
            stmt_key++;
        char *stmt_end = strstr(stmt_key, "\"");
        if (stmt_end != NULL)
        {
            char tmp = *stmt_end;
            *stmt_end = '\0';
            if (strlen(stmt_key) >= MAX_CHARACTERS)
            {
                perror("stmt overflow");
                *stmt_end = tmp;
                return 1;
            }
            strncpy(req->stmt, stmt_key, MAX_CHARACTERS - 1);
            req->stmt[MAX_CHARACTERS - 1] = '\0';
            *stmt_end = tmp;
        }
    }
    else
    {
        if (strncmp(req->path, "/sql", strlen("/sql")) == 0)
        {
            printf("request body missing key 'stmt'\n");
            return 1;
        }
        else
        {

            return 0;
        }
    }
    return 0;
}
int extract_target(Request *request, char *data)
{
    char *line_end = strstr(data, "\r\n");
    if (line_end == NULL)
    {
        printf("Invalid HTTP request\n");
        return 1;
    }
    size_t line_length = line_end - data;
    char *line = (char *)malloc(line_length + 1);
    if (line == NULL)
    {
        perror("malloc");
        return 1;
    }
    strncpy(line, data, line_length);
    line[line_length] = '\0';
    char *method = strtok(line, " ");
    if (method != NULL)
    {
        strncpy(request->method, method, MAX_CHARACTERS - 1);
        request->method[MAX_CHARACTERS - 1] = '\0';
    }

    char *path = strtok(NULL, " ");
    if (path != NULL)
    {
        strncpy(request->path, path, MAX_CHARACTERS - 1);
        request->path[MAX_CHARACTERS - 1] = '\0';
    }
    free(line);
    return 0;
}
int read_bytes(int socket, char **data)
{
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    ssize_t total_bytes_received = 0;
    ssize_t content_length = -1;
    char *header_end = NULL;

    while (1)
    {
        ssize_t bytes_received = recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
        {
            break;
        }

        *data = realloc(*data, total_bytes_received + bytes_received + 1);
        if (*data == NULL)
        {
            break;
        }
        buffer[bytes_received] = '\0';
        memcpy(*data + total_bytes_received, buffer, bytes_received);
        total_bytes_received += bytes_received;
        (*data)[total_bytes_received] = '\0';

        if (header_end == NULL)
        {
            header_end = strstr(*data, "\r\n\r\n");
            if (header_end != NULL)
            {
                header_end += 4;
                char *content_length_str = strstr(*data, "Content-Length: ");
                if (content_length_str == NULL)
                {
                    content_length_str = strstr(*data, "Content-length: ");
                }
                if (content_length_str == NULL)
                {
                    content_length_str = strstr(*data, "content-length: ");
                }
                if (content_length_str != NULL)
                {
                    sscanf(content_length_str, "%*s %zd", &content_length);
                    content_length += header_end - *data;
                }
                else
                {
                    content_length = total_bytes_received;
                }
            }
        }

        if (header_end != NULL && total_bytes_received >= content_length)
        {
            break;
        }
    }

    return 0;
}
int read_request(int client_socket, Request *request, char **data)
{
    read_bytes(client_socket, data);
    if (extract_target(request, *data) != 0 || parse_body(request, *data) != 0)
    {
        return 1;
    }
    return 0;
}
int proxy_request(int client_socket, Request *request, char *data, char **response)
{
    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(3000);
    struct hostent *host;
    host = gethostbyname("localhost");
    if (host == NULL)
    {
        perror("gethostbyname");
        char message[] = "mintdb pool - Error getting target host name";
        send_internal_error(client_socket, message);
        return -1;
    }
    memcpy(&target_addr.sin_addr, host->h_addr_list[0], host->h_length);
    int target_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (target_socket < 0)
    {
        printf("error creating target socket\n");
        char message[] = "mintdb pool - Error creating target socket";
        send_internal_error(client_socket, message);
        return -1;
    }
    if (connect(target_socket, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
    {
        perror("proxy connect");
        close(target_socket);
        char message[] = "mintdb pool - Error connecting to target";
        send_internal_error(client_socket, message);
        return 1;
    }
    size_t total_sent = 0;
    size_t request_length = strlen(data);
    while (total_sent < request_length)
    {
        ssize_t sent = send(target_socket, data + total_sent, request_length - total_sent, 0);
        if (sent == -1)
        {
            break;
        }
        total_sent += sent;
    }
    if (read_bytes(target_socket, response) != 0)
    {
        close(target_socket);
        printf("error reading target bytes\n");
        char message[] = "mintdb pool - Error reading target response";
        send_internal_error(client_socket, message);
        free(*response);
        *response = NULL;
        return 1;
    }
    close(target_socket);
    printf("returning response to client\n");
    if (send_bytes(client_socket, *response) != 0)
    {
        printf("error sending client bytes\n");
        free(*response);
        *response = NULL;
        return 1;
    }

    return 0;
}
void cleanup(int socket, Request *request, char *data, char *response)
{
    close(socket);
    if (request != NULL)
    {
        free(request);
    }
    if (data != NULL)
    {
        free(data);
    }
    if (response != NULL)
    {
        free(response);
    }
}
void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg);
    char *data = NULL;
    char *response = NULL;
    Request *request = (Request *)malloc(sizeof(Request));
    if (request == NULL)
    {
        perror("malloc");
        char message[] = "mintdb pool - Internal Error";
        send_internal_error(client_socket, message);
        cleanup(client_socket, request, data, response);
        return NULL;
    }
    if (read_request(client_socket, request, &data) != 0)
    {
        printf("error reading request\n");
        char message[] = "mintdb pool - Error reading request";
        send_internal_error(client_socket, message);
        cleanup(client_socket, request, data, response);
        return NULL;
    }
    printf("@\x1b[38;5;50m%s \x1b[0m%s\n", request->method, request->path);
    proxy_request(client_socket, request, data, &response);
    cleanup(client_socket, request, data, response);
    return NULL;
}
int server(int port)
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("error initializing socket");
        return 1;
    }
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
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1)
        {
            perror("failed to accept connection");
            exit(EXIT_FAILURE);
        }
        int *client_socket_ptr = (int *)malloc(sizeof(int));
        *client_socket_ptr = client_socket;
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client_socket_ptr) != 0)
        {
            perror("pthread_create");
            close(client_socket);
            free(client_socket_ptr);
            continue;
        }
        pthread_detach(thread);
    }
    close(server_socket);
    return 0;
}