#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "pool.h"

typedef struct
{
    void (*function)(void *);
    void *data;
} Task;

Task task_queue[MAX_TASKS];
int queue_start = 0;
int queue_end = 0;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

void *thread_function(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&queue_mutex);

        while (queue_start == queue_end)
        {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }

        Task task = task_queue[queue_start];
        queue_start = (queue_start + 1) % MAX_TASKS;

        pthread_mutex_unlock(&queue_mutex);

        task.function(task.data);
    }
}
void add_task_to_pool(void (*function)(void *), void *data)
{
    pthread_mutex_lock(&queue_mutex);

    task_queue[queue_end].function = function;
    task_queue[queue_end].data = data;
    queue_end = (queue_end + 1) % MAX_TASKS;

    pthread_cond_signal(&queue_cond);

    pthread_mutex_unlock(&queue_mutex);
}