#ifndef mintdb_pool_pool_h
#define MAX_TASKS 1000
#define THREAD_POOL_SIZE 8
void *thread_function(void *arg);
void add_task_to_pool(void (*function)(void *), void *data);
#endif