#include "solv_eq.h"

#include <mpi/mpi.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct task_info
{
    uint32_t start_pos;
    uint32_t points;
} task_info;

double a_    = 0.0;
double tau_  = 0.0;  //  0 < t < tau*M
double h_    = 0.0;  //  0 < x < h*N

uint32_t M_  = 0U;
uint32_t N_  = 0U;

double** global_solution = NULL;

double (*func_)(double, double) = NULL; // f(t,x)
double (*phi_)(double)          = NULL; // u(0,x) = phi(x) 
double (*psi_)(double)          = NULL; // u(t,0) = psi(t)

bool param_seted = false;

double** generate_data(uint32_t, uint32_t);
task_info* set_tasks(int rank, int num_procs);
void run_solution(int rank, int num_procs, double** data, task_info *points);
double** get_tasks(int rank, int num_procs, double** data, task_info *info);

void set_param(
    double a,
    double tau,     //  0 < t < tau*M
    double h,       //  0 < x < h*N
    uint32_t M,
    uint32_t N ,
    double (*func)(double, double), // f(t,x)
    double (*phi)(double),          // u(0,x) = phi(x) 
    double (*psi)(double))          // u(t,0) = psi(t)
{
    a_ = a;
    tau_ = tau;
    h_ = h;

    M_ = M;
    N_ = N;

    func_ = func;
    phi_ = phi;
    psi_ = psi;

    param_seted = true;
}

void solution(
    int argc,
    char* argv[])
{
    int rank;
    int num_procs;

    double start_all_time, end_all_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0)
        start_all_time = MPI_Wtime();

    task_info* info = set_tasks(rank, num_procs);

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
        for (int i = 1; i < num_procs; ++i)
        {
            MPI_Request request;
            uint32_t send_buf[2] = {info[i].points, info[i].start_pos};
            MPI_Isend(send_buf, 2, MPI_UINT32_T, i, i, MPI_COMM_WORLD, &request);
        }
    else
    {
        info = (task_info*) malloc(sizeof(task_info));

        uint32_t recv_buf[2];
        MPI_Recv(recv_buf, 2, MPI_UINT32_T, 0, rank, MPI_COMM_WORLD, NULL);
        info->points = recv_buf[0];
        info->start_pos = recv_buf[1];
    }

    double** data = generate_data(M_ + 1, info->points);
    
    double start_time, end_time;
    if (rank == 0)
        start_time = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);
    run_solution(rank, num_procs, data, info);
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        end_time = MPI_Wtime();
        printf("solution: %d %d:\n", M_, N_);
        printf("\tprocs: %d\n", num_procs);
        printf("\ttime: %lf\n", end_time - start_time);
    }
    global_solution = get_tasks(rank, num_procs, data, info);

    free(info);

    if (rank == 0)
    {
        end_all_time = MPI_Wtime();
        printf("solution: %d %d:\n", M_, N_);
        printf("\tprocs: %d\n", num_procs);
        printf("\tall time: %lf\n", end_all_time - start_all_time);
        save_solution();
    }

    MPI_Finalize();
}

void save_solution()
{
    const char* file_name = "solution1.txt";

    FILE* fd = fopen(file_name, "w");

    if (!fd)
    {
        printf("Cannot open file %s\n", file_name);
        return;
    }

    for (uint32_t i = 0; i <= M_; ++i)
    {
        for (uint32_t j = 0; j <= N_; ++j)
            fprintf(fd, "%lf ", global_solution[i][j]);
        
        fprintf(fd, "\n");
        free(global_solution[i]);
    }
    free(global_solution);

    fclose(fd);

    param_seted = false;
}

double** generate_data(uint32_t M, uint32_t N)
{
    double** data = (double**) calloc(M, sizeof(double*));
    if (!data) return NULL;

    for (uint32_t i = 0; i < M; ++i)
    {
        data[i] = (double*) calloc(N, sizeof(double));
        if (!data[i])
        {
            for (uint32_t j = 0; j < i; ++j)
                free(data[j]);
            
            free(data);
            return NULL;
        }
    }
    return data;
}

task_info* set_tasks(int rank, int num_procs)
{
    task_info* info = NULL;

    if (rank == 0)
    {
        info = (task_info*) malloc(num_procs *sizeof(task_info));

        uint32_t step = (N_ + 1) / num_procs;
        for (uint32_t i = 0; i < num_procs; ++i)
        {
            if (i == 0)
                info[0].start_pos = 0;
            else
                info[i].start_pos = info[i-1].start_pos + info[i-1].points;

            info[i].points = step;
            if (i == num_procs - 1)
                info[i].points = (N_ + 1) - info[i].start_pos;
        }
    }
    return info;
}

void run_solution(int rank, int num_procs, double** data, task_info *info)
{
    uint32_t start = info->start_pos;
    uint32_t end = start + info->points;
    uint32_t range = end - start;

    // Начальные условия
    for (uint32_t i = 0; i < range; ++i)
        data[0][i] = phi_((start + i) * h_);


    for (uint32_t i = 0; i < M_; ++i)
    {
        for (uint32_t j = 0; j < range; ++j)
        {
            double  left    = 0.0;
            double  right   = 0.0;
            double  down    = 0.0;
            
            // Отправка данных
            if ((j == 0) && (rank > 0))
            {
                MPI_Request request;
                MPI_Isend(&(data[i][j]), 1, MPI_DOUBLE, rank - 1, rank, MPI_COMM_WORLD, &request);
            }
            else
            if ((j == range - 1) && (rank < num_procs - 1))
            {
                MPI_Request request;
                MPI_Isend(&(data[i][j]), 1, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, &request);
            }

            // Полечение данных
            if (j == 0)
                if (rank == 0)
                    left = psi_(i*tau_);
                else
                    MPI_Recv(&left, 1, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD, NULL);

            else
                left = data[i][j-1];
            
            if (j == range - 1)
                if (rank == num_procs - 1)
                    right = 0.0; // TODO
                else
                    MPI_Recv(&right, 1, MPI_DOUBLE, rank + 1, rank + 1, MPI_COMM_WORLD, NULL);
            
            else
                right = data[i][j+1];
            
            if (i > 0)
                data[i+1][j] = 2 * tau_ *func_(i*tau_, (start + j)*h_) +
                    data[i-1][j] + a_ * (tau_ /h_) * (right - left);
            else
                data[i+1][j] = tau_ *func_(i*tau_, (start + j)*h_) +
                    data[i][j] + a_ * (tau_ /h_) * (right - left)/2;

            // Обработка данных
            
        }
    }
}

double** get_tasks(int rank, int num_procs, double** data, task_info *info)
{
    double** global_data = NULL;
    if (rank == 0)
    {
        global_data = generate_data(M_+1, N_+1);

        for (uint32_t i = 0; i <= M_; ++i)
        {
            for (uint32_t j = 0; j < info->points; ++j)
                global_data[i][j] = data[i][j];
            
            free(data[i]);
        }
        free(data);

        for (uint32_t proc = 1; proc < num_procs; ++proc)
            for (uint32_t i = 0; i <= M_; ++i)
                MPI_Recv(global_data[i] + info[proc].start_pos, info[proc].points, MPI_DOUBLE, proc, proc, MPI_COMM_WORLD, NULL);
    }
    else
    {
        for (uint32_t i = 0; i <= M_; ++i)
        {
            MPI_Send(data[i], info->points, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD);
            free(data[i]);
        }
        free(data);
    }

    return global_data;
}