#include "integr.hpp"
#include <cmath>
#include <iostream>
#include <functional>
#include <algorithm>
#include <string.h>
#include <iomanip>

#define TASK(num, l_x, r_x, code_expr, text_expr)           \
        case num:                                           \
            left = l_x;                                     \
            right = r_x;                                    \
            function = [] (double x) { return code_expr; }; \
            break;                                          \

void set_param(double &left, double &right, std::function<double(double)>& function, size_t &task_num)
{
    switch (task_num)
    {
        #include "tasks.hpp"
    default:
        left = 0.0;
        right = 1.0;
        task_num = 0;
        break;
    }
}
#undef TASK

#define FORMAT_LENGTH 100

size_t get_length(double eps)
{
    size_t lenght = 0;
    while (eps < 1)
    {
        lenght++;
        eps *= 10;
    }
    return std::max<size_t>(2U, lenght);
}

int parse_arg(int argc, char* argv[], double &eps, size_t &task_num, size_t &num_threads)
{
    if (argc < 2)
        return 1;
    
    if (strncmp("-e", argv[0], 3) == 0)
    {
        double eps_ = atof(argv[1]);
        if (eps_ <= 0.0) return 1;
        eps = eps_;

    } else
    if (strncmp("-t", argv[0], 3) == 0)
    {
        int task_num_ = atoi(argv[1]);
        if (task_num_ < 0) return 1;
        task_num = task_num_; 

    }
    else
    if (strncmp("-p", argv[0], 3) == 0)
    {
        int num_threads_ = atoi(argv[1]);
        if (num_threads_ <= 0) return 1;
        num_threads = num_threads_;

    } else
        return 1;

    if (argc > 2)
        return parse_arg(argc-2, argv+2, eps, task_num, num_threads);
    
    return 0;
}

#define TASK(num, l_x, r_x, code_expr, text_expr)       \
    case num:                                           \
        printf(format, left, right, text_expr, result); \
        break;

int main(int argc, char* argv[])
{
    double left = 0.0;
    double right = 1.0;
    double eps = 1e-4;
    std::function<double(double)> function = [](double x) { return x*x; };
    size_t task_num = 0;
    size_t num_threads = 0;

    if (parse_arg(argc -1, argv+1, eps, task_num, num_threads))
    {
        std::cout << "Wront input format!" << std::endl;
        return -1;
    }
    set_param(left, right, function, task_num);
    if (!task_num)
    {
        std::cout << "This task doesn't exist!" << std::endl;
        return -1;
    }

    integr::Integr J(left, right, eps, function);
    if (num_threads) J.set_num_threads(num_threads);

    auto start = std::chrono::high_resolution_clock::now();
    double result = J.get_result();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    char format[FORMAT_LENGTH] = {0};
    sprintf(format, "Int_ %%.4lf ^%%.4lf { %%s dx } = %%.%zulf\n", get_length(eps));

    switch (task_num)
    {
        #include "tasks.hpp"
    default:
        break;
    }

    std::cout << "Num threads: " << J.get_num_threads() << std::endl;
    std::cout << "Total time: " << duration.count() << " us" << std::endl;
    return 0;
}

#undef TASK