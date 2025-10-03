#include <mpi/mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int rank;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("Hello, World! rank: %d\n", rank);

    MPI_Finalize();
}