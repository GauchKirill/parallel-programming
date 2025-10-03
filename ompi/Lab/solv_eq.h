#ifndef SOLV_EQ_H
#define SOLV_EQ_H

#include <inttypes.h>

void set_param(
    double a,
    double tau,     //  0 < t < tau*M
    double h,       //  0 < x < h*N
    uint32_t M,
    uint32_t N ,
    double (*func)(double, double), // f(t,x)
    double (*phi)(double),          // u(0,x) = phi(x) 
    double (*psi)(double));         // u(t,0) = psi(t)

void solution(
    int argc,
    char* argv[]);

void save_solution();
#endif
