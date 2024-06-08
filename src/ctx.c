#include "config.h"
#include "lb.h"
#include "pool.h"
#include "ctx.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    pthread_t *pool;
} Context;

Context *ctx = NULL;

char *ctx_lb_host(char *path, char *stmt)
{
    int replica = lb_get_host(path, stmt);
    char *host = (char *)malloc(MAX_CHAR * sizeof(char));
    sprintf(host, "%s-%d", config->target_host_prefix, replica);
    return host;
}
int ctx_init()
{
    init_config();
    init_lb(config->replicas);
    pthread_t *pool = (pthread_t *)malloc(THREAD_POOL_SIZE * sizeof(pthread_t));
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_create(&pool[i], NULL, thread_function, NULL);
    }
    ctx = (Context *)malloc(sizeof(Context));
    ctx->pool = pool;
    return 0;
}
void ctx_add_task(void (*function)(void *), void *data)
{
    add_task_to_pool(function, data);
}
void ctx_cleanup()
{
    config_cleanup();
    lb_cleanup();
    thread_cleanup(ctx->pool);
    free(ctx->pool);
    free(ctx);
}