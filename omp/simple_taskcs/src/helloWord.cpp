#include <omp.h>
#include <iostream>

int main()
{
    #pragma omp parallel
    {
        int num = omp_get_thread_num();
        printf("Hello word! I'm %d\n", num);
    }   
}