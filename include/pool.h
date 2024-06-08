#ifndef mintdb_pool_pool_h
#define MAX_TASKS 10000
#define THREAD_POOL_SIZE 8

void *thread_function(void *arg);
void add_task_to_pool(void (*function)(void *), void *data);
void thread_cleanup(pthread_t *threads);

#endif