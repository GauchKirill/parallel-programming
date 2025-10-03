#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int rank, size;
    long long N;
    double global_sum = 0.0;
    double t_start, t_end;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    N = atoll(argv[1]);

    t_start = MPI_Wtime();
    
    double local_sum = 0.0;

    for (long long num = rank+1; num <= N; num += size)
        local_sum += 1.0 / num;

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    t_end = MPI_Wtime();
    
    if (rank == 0) {
        printf("1 + ... + 1/%lld = %.10lf\n", N, global_sum);
        printf("Total time: %lf\n", t_end-t_start);
    }
    
    MPI_Finalize();
    return 0;
}