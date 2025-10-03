#include <iostream>
#include <omp.h>

// Глобальная переменная, которая будет приватной для каждого потока
int global_var = 10;
#pragma omp threadprivate(global_var)

int main() {
    std::cout << "=== COPYIN ===" << std::endl;
    
    // Изменяем значение в главном потоке
    global_var = 100;
    std::cout << "Главный поток до parallel: global_var = " << global_var << std::endl;
    
    #pragma omp parallel copyin(global_var)
    {
        int thread_id = omp_get_thread_num();
        #pragma omp critical
        std::cout << "Поток " << thread_id << ": global_var = " << global_var << std::endl;
        
        // Каждый поток изменяет свою копию
        global_var += thread_id + 1;
        #pragma omp critical
        std::cout << "Поток " << thread_id << " после изменения: global_var = " << global_var << std::endl;
    }
    
    std::cout << "Главный поток после parallel: global_var = " << global_var << std::endl;
    
    return 0;
}