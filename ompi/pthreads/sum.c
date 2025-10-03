#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#define DEFAULT_NUM_THREADS 8U
#define MIN_POINTS_FOR_THREAD 10U

typedef struct thr_data_
{
    uint32_t start_num;
    uint32_t end_num;
    double sub_sum;
} thread_data;

void* thr_func(void* arg)
{
    thread_data* data = arg;
    data->sub_sum = 0.0;

    for (uint32_t n = data->start_num; n < data->end_num; ++n)
    {
        data->sub_sum += 1.0 / (double) n;
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    uint16_t num_threads;
    uint32_t N;

    if (argc > 2)
    {
        N = atoi(argv[1]);
        num_threads = atoi(argv[2]);
    }
    else if (argc == 2)
    {
        N = atoi(argv[1]);
        num_threads = DEFAULT_NUM_THREADS;
    }
    else
    {
        printf("Please, get N and \n");
        return 0;
    }

    if (num_threads > N * MIN_POINTS_FOR_THREAD)
        num_threads = N / MIN_POINTS_FOR_THREAD;

    pthread_t* thr;
    thr = (pthread_t*) malloc(num_threads * sizeof(pthread_t));

    thread_data* thr_data;
    thr_data = (thread_data*) malloc(num_threads * sizeof(thread_data));

    uint32_t step = N / num_threads;
    for (uint16_t i = 0; i < num_threads; ++i)
    {
        thr_data[i].start_num = i*step + 1;

        if (i == num_threads -1)
            thr_data[i].end_num = N + 1U;
        else
            thr_data[i].end_num = (i+1)*step + 1;

        pthread_create(thr + i, NULL, thr_func, &thr_data[i]);
    }

    double sum = 0;

    for (uint16_t i = 0; i < num_threads; ++i)
    {
        pthread_join(thr[i], NULL);
        sum += thr_data[i].sub_sum;
    }

    printf("N = %d\n", N);
    printf("1 + 1/2 + ... + 1/N = %.6lf\n", sum);
    return 0;
}