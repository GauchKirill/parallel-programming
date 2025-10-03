#include <omp.h>
#include <iostream>

int main()
{
    int sharedValue = 0;
    int numThreads = omp_get_max_threads();
    #pragma omp parallel for ordered shared(sharedValue) num_threads(numThreads)
    for (int i = 0; i < numThreads; ++i)
    {
        int threadId = omp_get_thread_num();

        #pragma omp ordered
        {
            sharedValue++;
            std::cout << "Thread Id: " << threadId <<
                        "; value: " << sharedValue << std::endl;
        }
    }
    return 0;
}