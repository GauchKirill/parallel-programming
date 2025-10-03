#include <stdio.h>
#include <stdlib.h>
#include "solv_eq.h"
#include <mpi/mpi.h>

double x_max    = 10.0;
double t_max    = 1.0;
double a        = 0.5;

uint32_t M = 1e5;
uint32_t N = 1e3;

double func(double t, double x)
{
    return 0.0;
}

double phi(double x)
{
    return (x_max - x)/x_max *1e-1;
}

double psi(double t)
{
    return 1e-1;
}

int main(int argc, char* argv[])
{
    set_param(a, t_max/M, x_max/N, M, N, func, phi, psi);
    solution(argc, argv);

    return 0;
}