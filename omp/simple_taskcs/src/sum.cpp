#include <omp.h>
#include <iostream>
#include <cstdlib>
#include <cstdint>

int main(int argc, char* argv[])
{
    uint64_t N = 0;
    if (argc == 2)
        N = std::stoull(argv[1]);

    if (N == 0) return 0;

    double sum = 0.0;
    #pragma omp reduction (+,sum)
    {
        #pragma omp parallel for
        for (int i = 1; i <= N; ++i)
        {
            sum += 1/static_cast<double>(i);
        }
    }
    std::cout << "sum 1 +...+ 1/" << N << " = " << sum << std::endl;
    return 0;
}