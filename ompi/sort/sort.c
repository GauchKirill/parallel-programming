#include "sort.h"

int main(int argc, char *argv[])
{
    int rank, num_procs;
    int global_size = (argc > 1) ? atoi(argv[1]) : DEFAULT_SIZE; // Размер глобального массива
    int *global_arr = NULL;
    int *sorted_global_arr = NULL;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0)
    {
        global_arr = (int *)malloc(global_size * sizeof(int));
        if (global_arr == NULL)
        {
            fprintf(stderr, "Memory allocation failed for global array\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Выберите тип заполнения массива для лучшего, среднего и худшего случаев
        fill_array_random(global_arr, global_size); // Типовой случай
        // fill_array_ascending(global_arr, global_size); // Лучший случай
        // fill_array_descending(global_arr, global_size); // Худший случай
    }

    MPI_Barrier(MPI_COMM_WORLD); // Синхронизация перед началом сортировки

    if (rank == 0)
        start_time = MPI_Wtime();
    sorted_global_arr = parallel_merge_sort(global_arr, global_size, rank, num_procs);
    if (rank == 0)
        end_time = MPI_Wtime();

    if (rank == 0)
    {
        printf("Time: %f s\n", end_time - start_time);
        printf("Elems: %d\n", global_size);
        printf("Num procs: %d\n\n", num_procs);

        // Вывод отсортированного массива
        if (global_size <= PRINT_SIZE)
        {
            for (int i = 0; i < global_size; i++)
            {
                printf("%d ", sorted_global_arr[i]);
            }
            printf("\n");
        }
        free(global_arr);
        free(sorted_global_arr);
    }

    MPI_Finalize();
    return 0;
}