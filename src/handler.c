#include "handler.h"

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
void write_response(char **res, const char * http_header, char *content_type, char *body)
{
    if (strcmp(http_header, HTTP_NO_CONTENT) == 0) {
        ssize_t response_size = strlen(http_header) + 1;
        *res = (char *)malloc(response_size * sizeof(char));
        snprintf(*res, response_size, "%s", http_header);
        return;
    }
    ssize_t response_size = r_size(http_header, content_type, body);
    *res = (char *)malloc(response_size * sizeof(char));
    snprintf(*res, response_size - 1, http_header, content_type, strlen(body), body);
}