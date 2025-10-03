#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#define DEFAULT_NUM_THREADS 8U

typedef struct thr_data_
{
    uint16_t tid;
} thread_data;

uint32_t global_num = 0;
pthread_mutex_t lock_num;

void* thr_func(void* arg)
{
    thread_data* data = arg;
    
    while (1)
    {
        pthread_mutex_lock(&lock_num);

        if (global_num == data->tid)
        {
            printf("Num before me: %d; My tid: %d\n", global_num, data->tid);
            global_num++;
            pthread_mutex_unlock(&lock_num);
            break;
        }
        else
            pthread_mutex_unlock(&lock_num);
    }

    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char* argv[])
{
    uint16_t num_threads;

    if (argc > 1)
        num_threads = atoi(argv[1]);
    else
        num_threads = DEFAULT_NUM_THREADS;

    pthread_t* thr;
    thr = (pthread_t*) malloc(num_threads * sizeof(pthread_t));

    thread_data* thr_data;
    thr_data = (thread_data*) malloc(num_threads * sizeof(thread_data));

    pthread_mutex_init(&lock_num, NULL);
    for (uint16_t i = 0; i < num_threads; ++i)
    {
        thr_data[i].tid = i;

        pthread_create(thr + i, NULL, thr_func, &thr_data[i]);
    }

    for (uint16_t i = 0; i < num_threads; ++i)
        pthread_join(thr[i], NULL);
    
    return 0;
}