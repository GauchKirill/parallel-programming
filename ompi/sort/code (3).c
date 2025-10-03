
#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
#include <time.h>

int compare(const void *a, const void *b);

// Функция для обмена данными между процессами
void exchange_data(int *local_arr, int local_size, int rank, int num_procs, int *global_arr, int global_size) {
    int *recvcounts = NULL;
    int *displs = NULL;

    if (rank == 0) {
        recvcounts = (int *)malloc(num_procs * sizeof(int));
        displs = (int *)malloc(num_procs * sizeof(int));

        for (int i = 0; i < num_procs; i++) {
            recvcounts[i] = global_size / num_procs;
            if (i < global_size % num_procs) {
                recvcounts[i]++;
            }
            displs[i] = (i == 0) ? 0 : displs[i - 1] + recvcounts[i - 1];
        }
    }

    // Отправляем данные из глобального массива в локальные массивы
    MPI_Scatterv(global_arr, recvcounts, displs, MPI_INT, local_arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        free(recvcounts);
        free(displs);
    }
}


// Функция для слияния двух отсортированных массивов
int* merge(int *arr1, int size1, int *arr2, int size2) {
    int *result = (int *)malloc((size1 + size2) * sizeof(int));
    int i = 0, j = 0, k = 0;

    while (i < size1 && j < size2) {
        if (arr1[i] <= arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }

    while (i < size1) {
        result[k++] = arr1[i++];
    }

    while (j < size2) {
        result[k++] = arr2[j++];
    }

    return result;
}

// Функция для параллельной сортировки слиянием
int* parallel_merge_sort(int *global_arr, int global_size, int rank, int num_procs) {
    int local_size = global_size / num_procs;
    if (rank < global_size % num_procs) {
        local_size++;
    }

    int *local_arr = (int *)malloc(local_size * sizeof(int));

    // Обмен данными между процессами (разделение массива)
    exchange_data(local_arr, local_size, rank, num_procs, global_arr, global_size);

    // Сортировка локального массива (можно использовать любой алгоритм, например, qsort)
    qsort(local_arr, local_size, sizeof(int), compare);

    // Сборка отсортированных локальных массивов
    int *recvcounts = NULL;
    int *displs = NULL;
    int *global_sorted_arr = NULL;
    int *local_sorted_arr = NULL;

    if (rank == 0) {
        recvcounts = (int *)malloc(num_procs * sizeof(int));
        displs = (int *)malloc(num_procs * sizeof(int));

        for (int i = 0; i < num_procs; i++) {
            recvcounts[i] = global_size / num_procs;
            if (i < global_size % num_procs) {
                recvcounts[i]++;
            }
            displs[i] = (i == 0) ? 0 : displs[i - 1] + recvcounts[i - 1];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Отправляем размер локального массива на процесс 0
    MPI_Gather(&local_size, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        displs[0] = 0;
        for (int i = 1; i < num_procs; i++) {
            displs[i] = displs[i - 1] + recvcounts[i - 1];
        }
        global_sorted_arr = (int *)malloc(global_size * sizeof(int));
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Gatherv(local_arr, local_size, MPI_INT, global_sorted_arr, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    // Parallel Merge (on rank 0)

    if (rank == 0) {

        int num_sublists = num_procs; // Each process has a sorted sublist
        int *starts = (int*)malloc(num_sublists * sizeof(int));
        int *sizes = (int*)malloc(num_sublists * sizeof(int));
        for(int i = 0; i < num_sublists; i++){
            starts[i] = displs[i];
            sizes[i] = recvcounts[i];
        }

        while (num_sublists > 1) {
            int new_num_sublists = (num_sublists + 1) / 2;  // divide sublists into pairs
            int *new_starts = (int*)malloc(new_num_sublists * sizeof(int));
            int *new_sizes = (int*)malloc(new_num_sublists * sizeof(int));


            for (int i = 0; i < new_num_sublists; i++) {

                if (2 * i + 1 < num_sublists) {
                    //Merge two sublists
                    int *merged = merge(global_sorted_arr + starts[2 * i], sizes[2 * i], global_sorted_arr + starts[2 * i + 1], sizes[2 * i + 1]);
                    int merged_size = sizes[2*i] + sizes[2*i+1];

                    //Copy merged elements to original global_sorted_arr
                    for(int j = 0; j < merged_size; j++){
                        global_sorted_arr[starts[2*i] + j] = merged[j];
                    }
                    free(merged);
                    new_starts[i] = starts[2 * i];
                    new_sizes[i] = merged_size;
                 } else { // Copy leftover
                    new_starts[i] = starts[2 * i];
                    new_sizes[i] = sizes[2 * i];
                }

            }

            free(starts);
            free(sizes);
            starts = new_starts;
            sizes = new_sizes;
            num_sublists = new_num_sublists;

        }
    }


    MPI_Barrier(MPI_COMM_WORLD);
    free(local_arr);

    if(rank == 0){
        free(recvcounts);
        free(displs);
    }


    return global_sorted_arr;
}


// Функция сравнения для qsort
int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

// Функция для заполнения массива случайными числами
void fill_array_random(int *arr, int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 1000; // случайные числа от 0 до 999
    }
}

// Функция для заполнения массива в порядке возрастания
void fill_array_ascending(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i;
    }
}

// Функция для заполнения массива в порядке убывания
void fill_array_descending(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = size - i - 1;
    }
}


int main(int argc, char *argv[]) {
    int rank, num_procs;
    int global_size = 10000; // Размер глобального массива
    int *global_arr = NULL;
    int *sorted_global_arr = NULL;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0) {
        global_arr = (int *)malloc(global_size * sizeof(int));
        if(global_arr == NULL){
            fprintf(stderr, "Memory allocation failed for global array\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Выберите тип заполнения массива для лучшего, среднего и худшего случаев
        fill_array_random(global_arr, global_size); // Типовой случай
        // fill_array_ascending(global_arr, global_size); // Лучший случай
        // fill_array_descending(global_arr, global_size); // Худший случай
    }

    MPI_Barrier(MPI_COMM_WORLD); // Синхронизация перед началом сортировки

    start_time = MPI_Wtime();
    sorted_global_arr = parallel_merge_sort(global_arr, global_size, rank, num_procs);
    end_time = MPI_Wtime();


    if (rank == 0) {
        printf("Time taken for parallel merge sort: %f seconds\n", end_time - start_time);

        // Вывод отсортированного массива (можно закомментировать для больших массивов)
         for (int i = 0; i < global_size; i++) {
           printf("%d ", sorted_global_arr[i]);
         }
         printf("\n");
        free(global_arr);
        free(sorted_global_arr);
    }

    MPI_Finalize();
    return 0;
}
