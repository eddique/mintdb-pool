#include "router.h"
#include <stdio.h>
#include <string.h>

int router(Request *req, char **res)
{
    if (strncmp(req->path, "/mintdb-pool/health", strlen("/mintdb-pool/health")) == 0)
    {
        health_handler(req, res);
    }
    else
    {
        proxy_handler(req, res);
    }
    return 0;
}