#ifndef mintdb_pool_lb_h
#include <pthread.h>
typedef struct {
    int replicas;
    int next;
    pthread_mutex_t mutex;
} LoadBalancer;
LoadBalancer *lb;
void init_lb(int replicas);
void lb_cleanup();
int lb_get_host(char *path, char *stmt);
#endif