#include "lb.h"
#include <stdlib.h>
#include <string.h>
LoadBalancer *lb = NULL;
void init_lb(int replicas)
{
    lb = (LoadBalancer *)malloc(sizeof(LoadBalancer));
    pthread_mutex_init(&lb->mutex, NULL);
    pthread_mutex_lock(&lb->mutex);
    lb->next = 0;
    lb->replicas = replicas;
    pthread_mutex_unlock(&lb->mutex);
}
int lb_next_replica()
{
    pthread_mutex_lock(&lb->mutex);
    int replica = lb->next;
    lb->next = (lb->next + 1) % lb->replicas;
    pthread_mutex_unlock(&lb->mutex);
    return replica;
}
int is_mutation(char *path, char *stmt)
{
    if (path != NULL && strncmp(path, "/sql", strlen("/sql")) == 0)
    {
        if (stmt != NULL)
        {
            if (strncmp(stmt, "insert", strlen("insert")) == 0)
            {
                return 0;
            }
            if (strncmp(stmt, "drop", strlen("drop")) == 0)
            {
                return 0;
            }
            if (strncmp(stmt, "delete", strlen("delete")) == 0)
            {
                return 0;
            }
            if (strncmp(stmt, "migrate", strlen("migrate")) == 0)
            {
                return 0;
            }

            if (strncmp(stmt, "batch", strlen("batch")) == 0)
            {
                return 0;
            }
        }
    }
    return 1;
}
int lb_get_host(char *path, char *stmt)
{
    if (is_mutation(path, stmt) == 0)
    {
        return 0;
    }
    return lb_next_replica();
}
void lb_cleanup()
{
    pthread_mutex_destroy(&lb->mutex);
    free(lb);
    lb = NULL;
}