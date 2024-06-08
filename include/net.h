#ifndef mintdb_pool_net_h
#include "ctx.h"

#define MAX_CHARACTERS 32
#define MAX_BUFFER_SIZE 8192
extern const char *HTTP_OK;
extern const char *HTTP_NO_CONTENT;
extern const char *HTTP_NOT_FOUND;
extern const char *HTTP_INTERNAL_ERROR;


typedef struct
{
    char method[MAX_CHARACTERS];
    char path[MAX_CHARACTERS];
    char stmt[MAX_CHARACTERS];
    char *data;
} Request;

int send_bytes(int socket, char *response);
int send_internal_error(int socket, char *error);
int read_bytes(int socket, char **data);
int extract_target(Request *request, char *data);
int parse_body(Request *req, char *data);
#endif