#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define ISIZE 5000
#define JSIZE 5000

double a[ISIZE][JSIZE];
double b[JSIZE][ISIZE];

void sequential_version()
{;
    int i, j;
    FILE *ff;

    // Инициализация
    for (i = 0; i < ISIZE; i++) {
        for (j = 0; j < JSIZE; j++) {
            a[i][j] = 10 * i + j;
        }
    }

    double t_start = omp_get_wtime();

    // Эталонная реализация
    // Антизависимость; (>,>)
    for (i = 0; i < ISIZE-2; i++) {
        for (j = 0; j < JSIZE-4; j++) {
            a[i][j] = sin(0.1 * a[i+2][j+4]);
        }
    }

    double t_end = omp_get_wtime();
    printf("OpenMP 2z time: %f sec\n", t_end - t_start);

    // Запись результата
    ff = fopen("result_2z_openmp_sequential.txt", "w");
    for (i = 0; i < ISIZE; i++) {
        for (j = 0; j < JSIZE; j++) {
            fprintf(ff, "%f ", a[i][j]);
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
        }
    }

    double t_start = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < ISIZE; ++i)
        for (int j = 0; j < JSIZE; ++j)
            b[i][j] = a[i][j];

    // Параллельная реализация
    #pragma omp parallel for
    for (int i = 0; i < ISIZE-2; ++i)
    {
        for (int j = 0; j < JSIZE-4; ++j)
        {
            a[i][j] = sin(0.1 * b[i+2][j+4]);
        }
    }

    double t_end = omp_get_wtime();
    printf("OpenMP 2z time: %f sec\n", t_end - t_start);

    // Запись результата
    ff = fopen("result_2z_openmp_parallel.txt", "w");
    for (int i = 0; i < ISIZE; i++) {
        for (int j = 0; j < JSIZE; j++) {
            fprintf(ff, "%f ", a[i][j]);
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