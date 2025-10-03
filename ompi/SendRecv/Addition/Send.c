#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DELAY 2 // задержка в секундах

int main(int argc, char **argv) {
    int rank;

    int msg_size = (argc > 1) ? atoi(argv[1]) : 1;
    char *buffer = malloc(msg_size);
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if (rank == 0) {
        // Процесс-отправитель
        // sleep(0.1);
        double t_start, t_end;

        int sendBufSize = MPI_BSEND_OVERHEAD + msg_size;
        char *bsend_buffer = malloc(sendBufSize);
        MPI_Buffer_attach(bsend_buffer, sendBufSize);

        t_start = MPI_Wtime();
        
        MPI_Bsend(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        
        t_end = MPI_Wtime();
        printf("Отправитель: MPI_Bsend: time - %f с; size - %d байт\n", t_end - t_start, msg_size);

        MPI_Buffer_detach(&bsend_buffer, &sendBufSize);
        free(bsend_buffer);

    } else if (rank == 1) {
        // Процесс-приемник

        sleep(DELAY);
        MPI_Recv(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Получатель: MPI_Recv: size - %d байт\n", msg_size);
    }
    
    free(buffer);
    
    MPI_Finalize();
    return 0;
}
