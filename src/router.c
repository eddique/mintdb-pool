#include "router.h"
#include <stdio.h>
#include <string.h>

int match_path(char *path, char *request_path)
{
    ssize_t path_len = strlen(path);
    return strncmp(request_path, path, path_len);
}

int router(Request *req, char **res)
{
    if (match_path("/mintdb-pool/health", req->path) == 0)
    {
        health_handler(req, res);
    }
    else
    {
        proxy_handler(req, res);
    }
    return 0;
}