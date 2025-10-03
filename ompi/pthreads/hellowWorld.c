#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#define DEFAULT_NUM_THREADS 4U

typedef struct thr_data_
{
    uint16_t tid;
} thread_data;

void* thr_func(void* arg)
{
    thread_data* data = arg;
    printf("Hello world! My tid: %d\n", data->tid);
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

    for (uint16_t i = 0; i < num_threads; ++i)
    {
        
        thr_data[i].tid = i;
        pthread_create(thr + i, NULL, thr_func, &thr_data[i]);
    }

    for (uint16_t i = 0; i < num_threads; ++i)
    {
        pthread_join(thr[i], NULL);
    }
}