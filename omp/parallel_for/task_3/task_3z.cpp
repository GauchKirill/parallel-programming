#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define ISIZE 5000
#define JSIZE 5000

double a[ISIZE][JSIZE];
double b[ISIZE][JSIZE];
double c[JSIZE][ISIZE];
double d[JSIZE][ISIZE];

void sequential_version()
{;
    FILE *ff;

    // Инициализация
    for (int i = 0; i < ISIZE; i++) {
        for (int j = 0; j < JSIZE; j++) {
            a[i][j] = 10 * i + j;
            b[i][j] = 0;
        }
    }

    double t_start = omp_get_wtime();

    // Эталонная реализация
    // 1. Нет зависимостей
    // 2. Истинная зависимость i, антизависимоть j; (<,>)
    for (int i = 0; i < ISIZE; i++) {
        for (int j = 0; j < JSIZE; j++) {
            a[i][j] = sin(0.1 * a[i][j]);
        }
    }
    for (int i = 1; i < ISIZE; ++i)
    {
        for (int j = 0; j < JSIZE-2; ++j)
        {
            b[i][j] = a[i-1][j+2]*1.5;
        }
    }
    double t_end = omp_get_wtime();
    printf("OpenMP 3z time: %f sec\n", t_end - t_start);

    // Запись результата
    ff = fopen("result_3z_openmp_sequential.txt", "w");
    for (int i = 0; i < ISIZE; i++) {
        for (int j = 0; j < JSIZE; j++) {
            fprintf(ff, "%f ", b[i][j]);
        }
        fprintf(ff, "\n");
    }
    fclose(ff);
}

void parallel_version()
{
    FILE *ff;
    
    // Инициализация
    for (int i = 0; i < ISIZE; i++) {
        for (int j = 0; j < JSIZE; j++) {
            a[i][j] = 10 * i + j;
            b[i][j] = 0;
        }
    }

    double t_start = omp_get_wtime();

    // Параллельная реализация
    #pragma omp parallel for
    for (int i = 0; i < ISIZE; i++) {
        for (int j = 0; j < JSIZE; j++) {
            a[i][j] = sin(0.1 * a[i][j]);
        }
    }
    #pragma omp parallel for
    for (int i = 0; i < ISIZE; ++i)
    {
        for (int j = 0; j < JSIZE; ++j)
        {
            c[j][i] = a[i][j];
            d[j][i] = b[i][j];
        }
    } 
    #pragma omp parallel for
    for (int j = 0; j < JSIZE-2; ++j)
    {
        for (int i = 1; i < ISIZE; ++i)
        {
            d[j][i] = c[j+2][i-1]*1.5;
        }
    }
    double t_end = omp_get_wtime();
    printf("OpenMP 3z time: %f sec\n", t_end - t_start);

    // Запись результата
    ff = fopen("result_3z_openmp_parallel.txt", "w");
    for (int i = 0; i < ISIZE; i++) {
        for (int j = 0; j < JSIZE; j++) {
            fprintf(ff, "%f ", d[j][i]);
        }
        fprintf(ff, "\n");
    }
    fclose(ff);
}

int main(int argc, char** argv) {
    int num_threads = 4;
    if (argc > 1)
        num_threads = atoi(argv[1]);
    omp_set_num_threads(num_threads);
    printf("Кол-во исполнителей: %d\n", num_threads);
    sequential_version();
    parallel_version();
    return 0;
}