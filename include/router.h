#ifndef mintdb_pool_router_h
#define mintdb_pool_router_h
#define MAX_CHARACTERS 32
typedef struct
{
    char method[MAX_CHARACTERS];
    char path[MAX_CHARACTERS];
    char stmt[MAX_CHARACTERS];
} Request;
int router(Request* request, char *data);
#endif