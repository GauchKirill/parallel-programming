#include <iostream>
#include <omp.h>

int main() {
    std::cout << "\n=== COPYPRIVATE ===" << std::endl;
    
    int shared_value = 0;  // Общая переменная для всех потоков
    
    std::cout << "shared_value = " << shared_value << std::endl;

    #pragma omp parallel firstprivate(shared_value)
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp single copyprivate(shared_value)
        {
            // Этот блок выполняется только одним потоком
            std::cout << "Поток " << thread_id << " вычисляет значение..." << std::endl;
            shared_value += thread_id;  // Некоторое вычисление
            std::cout << "Поток " << thread_id << " установил shared_value = " << shared_value << std::endl;
        }
        
        // Все потоки ждут завершения single-блока и получают значение
        #pragma omp barrier
        
        #pragma omp critical
        std::cout << "Поток " << thread_id << " получил shared_value = " << shared_value << std::endl;
    }
    
    return 0;
}