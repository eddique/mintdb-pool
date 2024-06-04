#ifndef mintdb_pool_handler_h
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
extern const char *HTTP_OK;
extern const char *HTTP_NO_CONTENT;
extern const char *HTTP_NOT_FOUND;
extern const char *HTTP_INTERNAL_ERROR;

void write_response(char **res, const char * http_header, char *content_type, char *body);

#endif