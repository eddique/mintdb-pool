#ifndef mintdb_pool_ctx_h

char *ctx_lb_host(char *path, char *stmt);
int ctx_init();
void ctx_add_task(void (*function)(void *), void *data);
void ctx_cleanup();
#endif