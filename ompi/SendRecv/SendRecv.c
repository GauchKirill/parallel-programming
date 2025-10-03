#include <mpi/mpi.h>
#include <stdio.h>

int main(int argc,char **argv)
{
    int rank;
    int size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size == 1)
    {
        MPI_Finalize();
        return 0;
    }

    int num = 1;
    if (rank == 0)
    {
        double t_start, t_end;
        
        t_start = MPI_Wtime();

        MPI_Send(&num, 1, MPI_INT, 1,       0, MPI_COMM_WORLD);
        printf("rank: %d; Send: num = %d\n", rank, num);
        
        MPI_Recv(&num, 1, MPI_INT, size-1,  0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("rank: %d; Recv: num = %d\n", rank, num);

        t_end = MPI_Wtime();

        MPI_Finalize();

        printf("process took: %lf\n", t_end - t_start);
    }
    else
    {
        MPI_Recv(&num, 1, MPI_INT, rank-1,          0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("rank: %d; Recv: num = %d\n", rank, num++);

        MPI_Send(&num, 1, MPI_INT, (rank+1) % size, 0, MPI_COMM_WORLD);
        printf("rank: %d; Send: num = %d\n", rank, num);

        MPI_Finalize();
    }

    return 0;
}