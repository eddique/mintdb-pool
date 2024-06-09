#ifndef mintdb_pool_ctx_h

int ctx_init();
int ctx_port();
int ctx_target_port();
char *ctx_lb_host(char *path, char *stmt);
void ctx_add_task(void (*function)(void *), void *data);
void ctx_cleanup();
#endif