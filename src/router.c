#include "router.h"
#include <stdio.h>
#include <string.h>

int router(Request *request, char *data)
{
    printf("in router\n");
    printf("method %s\n", request->method);
    printf("path %s\n", request->method);
    printf("stmt %s\n", request->stmt);
    printf("data %lu\n", strlen(data));
    printf("received @%s -> %s action %s request_length %lu\n", request->method, request->path, request->stmt, strlen(data));
    return 0;
}