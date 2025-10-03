#include <omp.h>
#include <iostream>
#include <vector>

int main() {
    // Включаем вложенный параллелизм
    omp_set_nested(1);
    
    // Устанавливаем количество потоков на каждом уровне
    omp_set_num_threads(2); // Уровень 1
    
    std::cout << "=== ВЛОЖЕННЫЙ ПАРАЛЛЕЛИЗМ ===" << std::endl;
    std::cout << "Максимальная вложенность: " << omp_get_max_active_levels() << std::endl;
    std::cout << "Вложенный параллелизм: " << (omp_get_nested() ? "включен" : "выключен") << std::endl << std::endl;

    #pragma omp parallel
    {
        int level1_thread = omp_get_thread_num();
        int level1_threads = omp_get_num_threads();
        
        // Устанавливаем количество потоков для второго уровня
        omp_set_num_threads(3); // Уровень 2
        
        #pragma omp parallel
        {
            int level2_thread = omp_get_thread_num();
            int level2_threads = omp_get_num_threads();
            
            // Устанавливаем количество потоков для третьего уровня
            omp_set_num_threads(2); // Уровень 3
            
            #pragma omp parallel
            {
                int level3_thread = omp_get_thread_num();
                int level3_threads = omp_get_num_threads();
                
                // Критическая секция для корректного вывода
                #pragma omp critical
                {
                    std::cout << "Уровень 3: поток " << level3_thread 
                              << "/" << level3_threads
                              << " (родитель: Ур.2 поток " << level2_thread
                              << "/" << level2_threads
                              << ", предок: Ур.1 поток " << level1_thread
                              << "/" << level1_threads << ")" << std::endl;
                }
            }
            
            #pragma omp critical
            {
                std::cout << "Уровень 2: поток " << level2_thread 
                          << "/" << level2_threads
                          << " (родитель: Ур.1 поток " << level1_thread
                          << "/" << level1_threads << ")" << std::endl;
            }
        }
        
        #pragma omp critical
        {
            std::cout << "Уровень 1: поток " << level1_thread 
                      << "/" << level1_threads << std::endl;
        }
    }

    std::cout << "\n=== ИНФОРМАЦИЯ О ФУНКЦИЯХ ===" << std::endl;
    
    // Демонстрация работы функций на разных уровнях
    omp_set_num_threads(2);
    #pragma omp parallel
    {
        #pragma omp critical
        std::cout << "Ур.1: omp_get_thread_num() = " << omp_get_thread_num()
                  << ", omp_get_num_threads() = " << omp_get_num_threads() << std::endl;
        
        omp_set_num_threads(3);
        #pragma omp parallel
        {
            #pragma omp critical
            std::cout << "  Ур.2: omp_get_thread_num() = " << omp_get_thread_num()
                      << ", omp_get_num_threads() = " << omp_get_num_threads() << std::endl;
            
            omp_set_num_threads(2);
            #pragma omp parallel
            {
                #pragma omp critical
                std::cout << "    Ур.3: omp_get_thread_num() = " << omp_get_thread_num()
                          << ", omp_get_num_threads() = " << omp_get_num_threads() << std::endl;
            }
        }
    }

    std::cout << "\n=== ФУНКЦИИ ДЛЯ ОПРЕДЕЛЕНИЯ КООРДИНАТ ===" << std::endl;
    
    // Демонстрация функций для определения координат в дереве потоков
    omp_set_num_threads(2);
    #pragma omp parallel
    {
        int level = 1;
        #pragma omp critical
        std::cout << "Уровень " << level << ": " 
                  << "thread_num = " << omp_get_thread_num()
                  << ", ancestor(0) = " << omp_get_ancestor_thread_num(0)
                  << ", team_size(0) = " << omp_get_team_size(0) << std::endl;
        
        omp_set_num_threads(3);
        #pragma omp parallel
        {
            int level = 2;
            #pragma omp critical
            std::cout << "  Уровень " << level << ": " 
                      << "thread_num = " << omp_get_thread_num()
                      << ", ancestor(0) = " << omp_get_ancestor_thread_num(0)
                      << ", ancestor(1) = " << omp_get_ancestor_thread_num(1)
                      << ", team_size(0) = " << omp_get_team_size(0)
                      << ", team_size(1) = " << omp_get_team_size(1) << std::endl;
            
            omp_set_num_threads(2);
            #pragma omp parallel
            {
                int level = 3;
                #pragma omp critical
                std::cout << "    Уровень " << level << ": " 
                          << "thread_num = " << omp_get_thread_num()
                          << ", ancestor(0) = " << omp_get_ancestor_thread_num(0)
                          << ", ancestor(1) = " << omp_get_ancestor_thread_num(1)
                          << ", ancestor(2) = " << omp_get_ancestor_thread_num(2)
                          << ", team_size(0) = " << omp_get_team_size(0)
                          << ", team_size(1) = " << omp_get_team_size(1)
                          << ", team_size(2) = " << omp_get_team_size(2) << std::endl;
            }
        }
    }

    return 0;
}