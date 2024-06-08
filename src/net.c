#include "net.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
const char *HTTP_OK =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %lu\r\n"
    "\r\n"
    "%s";
const char *HTTP_NOT_FOUND =
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %lu\r\n"
    "\r\n"
    "%s";
const char *HTTP_NO_CONTENT =
    "HTTP/1.1 204 No Content\r\n"
    "Content-Length: 0\r\n"
    "\r\n";
const char *HTTP_INTERNAL_ERROR =
    "HTTP/1.1 500 INTERNAL ERROR\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %lu\r\n"
    "\r\n"
    "%s";
    
ssize_t r_size(const char *http, char *content_type, char *data)
{
    ssize_t size = strlen(http);
    if (content_type != NULL)
    {
        size += strlen(content_type);
    }
    if (data != NULL)
    {
        size += strlen(data);
    }
    size += 50;
    return size;
}
void write_response(char **res, const char *http_header, char *content_type, char *body)
{
    if (strcmp(http_header, HTTP_NO_CONTENT) == 0)
    {
        ssize_t response_size = strlen(http_header) + 1;
        *res = (char *)malloc(response_size * sizeof(char));
        snprintf(*res, response_size, "%s", http_header);
        return;
    }
    ssize_t response_size = r_size(http_header, content_type, body);
    *res = (char *)malloc(response_size * sizeof(char));
    snprintf(*res, response_size - 1, http_header, content_type, strlen(body), body);
}
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