#ifndef mintdb_pool_handler_h
#include "net.h"

void write_response(char **res, const char * http_header, char *content_type, char *body);
int health_handler(Request *req, char **res);
int proxy_handler(Request *req, char **res);
#endif