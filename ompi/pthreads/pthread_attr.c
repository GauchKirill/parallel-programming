#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

#define DEFAULT_NUM_THREADS 4U

pthread_mutex_t mutex;

typedef struct thr_data_
{
    uint16_t tid;
} thread_data;

void* thr_func(void *arg)
{
    pthread_attr_t attr;
    int ret;
    thread_data* data = (thread_data*) arg;

    pthread_mutex_lock(&mutex);

    printf("\nMy tid: %d\n", data->tid);
    // Инициализируем структуру атрибутов потока
    ret = pthread_attr_init(&attr);
    if (ret != 0) {
        perror("pthread_attr_init");
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    // 1. Scope (область видимости)
    int scope;
    ret = pthread_attr_getscope(&attr, &scope);
    if (ret != 0) {
        perror("pthread_attr_getscope");
        pthread_attr_destroy(&attr); // Освобождаем структуру атрибутов в случае ошибки
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    printf("Scope: ");
    if (scope == PTHREAD_SCOPE_SYSTEM) {
        printf("PTHREAD_SCOPE_SYSTEM (System contention scope)\n");
    } else if (scope == PTHREAD_SCOPE_PROCESS) {
        printf("PTHREAD_SCOPE_PROCESS (Process contention scope)\n");
    } else {
        printf("Unknown scope value: %d\n", scope);
    }


    // 2. Detach State (состояние отсоединения)
    int detachstate;
    ret = pthread_attr_getdetachstate(&attr, &detachstate);
    if (ret != 0) {
        perror("pthread_attr_getdetachstate");
        pthread_attr_destroy(&attr);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    printf("Detach state: ");
    if (detachstate == PTHREAD_CREATE_DETACHED) {
        printf("PTHREAD_CREATE_DETACHED\n");
    } else if (detachstate == PTHREAD_CREATE_JOINABLE) {
        printf("PTHREAD_CREATE_JOINABLE\n");
    } else {
        printf("Unknown detach state value: %d\n", detachstate);
    }


    // 3. Stack Address (адрес стека) - Не рекомендуется использовать непосредственно.
    void *stackaddr;
    size_t stacksize_ret;

    ret = pthread_attr_getstack(&attr, &stackaddr, &stacksize_ret);
    if(ret != 0){
         perror("pthread_attr_getstack");
         pthread_attr_destroy(&attr);
         pthread_mutex_unlock(&mutex);
         return NULL;
    }
    
    printf("Stack address: %p\n", stackaddr);

    // 4. Stack Size (размер стека)
    size_t stacksize;
    ret = pthread_attr_getstacksize(&attr, &stacksize);
    if (ret != 0) {
        perror("pthread_attr_getstacksize");
        pthread_attr_destroy(&attr);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    printf("Stack size: %zu bytes\n", stacksize);


    // 5. Inherit Sched (наследование политики планирования)
    int inheritsched;
    ret = pthread_attr_getinheritsched(&attr, &inheritsched);
    if (ret != 0) {
        perror("pthread_attr_getinheritsched");
        pthread_attr_destroy(&attr);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    printf("Inherit Sched: ");
    if (inheritsched == PTHREAD_INHERIT_SCHED) {
        printf("PTHREAD_INHERIT_SCHED\n");
    } else if (inheritsched == PTHREAD_EXPLICIT_SCHED) {
        printf("PTHREAD_EXPLICIT_SCHED\n");
    } else {
        printf("Unknown inherit sched value: %d\n", inheritsched);
    }

    // 6. Sched Policy (политика планирования)
    int schedpolicy;
    struct sched_param schedparam;

    ret = pthread_attr_getschedpolicy(&attr, &schedpolicy);
    if (ret != 0) {
        perror("pthread_attr_getschedpolicy");
        pthread_attr_destroy(&attr);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    printf("Sched Policy: ");
    switch (schedpolicy) {
        case SCHED_FIFO:
            printf("SCHED_FIFO (First-In, First-Out)\n");
            break;
        case SCHED_RR:
            printf("SCHED_RR (Round-Robin)\n");
            break;
        case SCHED_OTHER:
            printf("SCHED_OTHER (Implementation-defined default scheduler)\n");
            break;
        default:
            printf("Unknown sched policy value: %d\n", schedpolicy);
    }


    // Уничтожаем структуру атрибутов потока, чтобы освободить выделенную память.
    ret = pthread_attr_destroy(&attr);
    if (ret != 0) {
        perror("pthread_attr_destroy");
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    pthread_mutex_unlock(&mutex);
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

    pthread_mutex_init(&mutex, NULL);
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
