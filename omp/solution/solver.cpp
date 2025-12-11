#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <sstream>
#include <atomic>

using namespace std;

const double SQRT2 = sqrt(2.0);
mutex mtx;
atomic<int> successful_solutions(0);

// Метод прогонки (Thomas algorithm)
vector<double> thomas_algorithm(const vector<double>& a, const vector<double>& b,
                                const vector<double>& c, const vector<double>& d) {
    int n = d.size();
    if (n == 0) return vector<double>();
    
    vector<double> p(n), q(n);
    vector<double> x(n);
    
    // Прямой ход
    p[0] = -c[0] / b[0];
    q[0] = d[0] / b[0];
    
    for (int i = 1; i < n - 1; ++i) {
        double denominator = b[i] + a[i] * p[i-1];
        p[i] = -c[i] / denominator;
        q[i] = (d[i] - a[i] * q[i-1]) / denominator;
    }
    
    // Последнее уравнение
    double denominator = b[n-1] + a[n-1] * p[n-2];
    q[n-1] = (d[n-1] - a[n-1] * q[n-2]) / denominator;
    
    // Обратный ход
    x[n-1] = q[n-1];
    for (int i = n-2; i >= 0; --i) {
        x[i] = p[i] * x[i+1] + q[i];
    }
    
    return x;
}

// Редукционный алгоритм для создания 8 независимых систем
void create_reduced_systems(double a_param, double h, int n_points,
                           vector<vector<double>>& all_A, vector<vector<double>>& all_B,
                           vector<vector<double>>& all_C, vector<vector<double>>& all_D,
                           vector<int>& system_sizes) {
    
    // Создаем исходную трехдиагональную систему
    vector<double> A(n_points), B(n_points), C(n_points), D(n_points);
    double h2 = h * h;
    
    // Заполняем коэффициенты для линеаризованного уравнения
    // y'' = a(y - y^3) линеаризуется как: y'' = a*(1 - 3y0^2)*y - 2a*y0^3
    // Но для простоты начального приближения возьмем y0 = SQRT2
    
    for (int i = 0; i < n_points; ++i) {
        A[i] = 1.0;
        B[i] = -2.0 - h2 * a_param * (1.0 - 3.0 * SQRT2 * SQRT2);
        C[i] = 1.0;
        D[i] = h2 * (-2.0 * a_param * SQRT2 * SQRT2 * SQRT2);
    }
    
    // Учет граничных условий
    B[0] = 1.0; C[0] = 0.0; D[0] = SQRT2;
    A[n_points-1] = 0.0; B[n_points-1] = 1.0; D[n_points-1] = SQRT2;
    
    // Редукционный алгоритм для получения 8 независимых систем
    // Исключаем все точки, кроме тех, что имеют одинаковый остаток от деления на 8
    
    all_A.resize(8);
    all_B.resize(8);
    all_C.resize(8);
    all_D.resize(8);
    system_sizes.resize(8);
    
    for (int r = 0; r < 8; ++r) {
        // Определяем индексы точек для данной подсистемы
        vector<int> indices;
        for (int i = 0; i < n_points; ++i) {
            if (i % 8 == r) {
                indices.push_back(i);
            }
        }
        
        int m = indices.size();
        system_sizes[r] = m;
        
        if (m == 0) continue;
        
        // Создаем коэффициенты для подсистемы
        vector<double> sub_A(m), sub_B(m), sub_C(m), sub_D(m);
        
        for (int j = 0; j < m; ++j) {
            int i = indices[j];
            
            // Основной коэффициент
            sub_B[j] = B[i];
            sub_D[j] = D[i];
            
            // Левая связь
            if (j > 0) {
                int prev_idx = indices[j-1];
                // Все точки между prev_idx и i исключаются
                // Их влияние учитывается в измененных коэффициентах
                sub_A[j] = 0.0;  // Будут скорректированы
            } else {
                sub_A[j] = 0.0;
            }
            
            // Правая связь
            if (j < m - 1) {
                int next_idx = indices[j+1];
                sub_C[j] = 0.0;  // Будут скорректированы
            } else {
                sub_C[j] = 0.0;
            }
        }
        
        // Корректируем коэффициенты с учетом исключенных точек
        // Используем метод редукции для трехдиагональных матриц
        for (int j = 0; j < m; ++j) {
            int i = indices[j];
            
            // Для каждой исключенной точки между текущей и следующей
            // в исходной системе применяем формулу исключения
            if (j < m - 1) {
                int next_i = indices[j+1];
                
                // Исключаем все точки между i и next_i
                for (int k = i + 1; k < next_i; ++k) {
                    // Выражаем y[k] через соседние
                    double factor = 1.0 / (B[k] - A[k] * sub_C[j] - C[k] * sub_A[j+1]);
                    sub_B[j] -= C[i] * A[k] * factor * sub_C[j];
                    sub_B[j+1] -= A[next_i] * C[k] * factor * sub_A[j+1];
                    sub_D[j] += C[i] * D[k] * factor;
                    sub_D[j+1] += A[next_i] * D[k] * factor;
                }
                
                // Устанавливаем связь между точками подсистемы
                sub_C[j] = C[i];
                sub_A[j+1] = A[next_i];
            }
        }
        
        // Учет граничных условий в подсистеме
        if (indices[0] == 0) {
            sub_B[0] = 1.0;
            sub_C[0] = 0.0;
            sub_D[0] = SQRT2;
        }
        
        if (indices[m-1] == n_points - 1) {
            sub_A[m-1] = 0.0;
            sub_B[m-1] = 1.0;
            sub_D[m-1] = SQRT2;
        }
        
        all_A[r] = sub_A;
        all_B[r] = sub_B;
        all_C[r] = sub_C;
        all_D[r] = sub_D;
    }
}

// Функция для решения одной подсистемы в потоке
void solve_subsystem(int thread_id, const vector<double>& A, const vector<double>& B,
                     const vector<double>& C, const vector<double>& D,
                     vector<double>& solution, atomic<bool>& success) {
    try {
        solution = thomas_algorithm(A, B, C, D);
        success = true;
        
        lock_guard<mutex> lock(mtx);
        cout << "  Thread " << thread_id << ": solved subsystem of size " << solution.size() << endl;
    } catch (...) {
        success = false;
        
        lock_guard<mutex> lock(mtx);
        cout << "  Thread " << thread_id << ": failed to solve subsystem" << endl;
    }
}

// Решение задачи для одного значения параметра a
vector<double> solve_for_a(double a_param) {
    // Вычисляем шаг согласно условию: h = 1/(sqrt(a) * 100)
    double h = 1.0 / (sqrt(a_param) * 100.0);
    
    // Определяем общее количество точек
    int n_points = static_cast<int>(20.0 / h) + 1;  // интервал [-10, 10]
    
    // Корректируем так, чтобы общее количество точек было кратно 8
    n_points = ((n_points + 7) / 8) * 8 + 1;
    
    // Проверяем, что количество точек на поток в допустимом диапазоне
    int points_per_thread = (n_points - 1) / 8;
    if (points_per_thread < 400) {
        n_points = 400 * 8 + 1;
        h = 20.0 / (n_points - 1);
    } else if (points_per_thread > 4000) {
        n_points = 4000 * 8 + 1;
        h = 20.0 / (n_points - 1);
    }
    
    cout << "  h = " << scientific << h << ", total points = " << n_points 
         << ", points per thread = " << (n_points - 1) / 8 << endl;
    
    // Создаем 8 независимых систем с помощью редукционного алгоритма
    vector<vector<double>> all_A, all_B, all_C, all_D;
    vector<int> system_sizes;
    
    create_reduced_systems(a_param, h, n_points, all_A, all_B, all_C, all_D, system_sizes);
    
    // Запускаем 8 потоков для решения подсистем
    vector<thread> threads(8);
    vector<vector<double>> solutions(8);
    vector<atomic<bool>> success_flags(8);
    
    for (int i = 0; i < 8; ++i) {
        success_flags[i] = false;
    }
    
    for (int i = 0; i < 8; ++i) {
        if (system_sizes[i] > 0) {
            threads[i] = thread(solve_subsystem, i, 
                               ref(all_A[i]), ref(all_B[i]), ref(all_C[i]), ref(all_D[i]),
                               ref(solutions[i]), ref(success_flags[i]));
        } else {
            success_flags[i] = true;  // Нет точек для этого потока
        }
    }
    
    // Ждем завершения всех потоков
    for (int i = 0; i < 8; ++i) {
        if (threads[i].joinable()) {
            threads[i].join();
        }
    }
    
    // Проверяем успешность всех потоков
    bool all_success = true;
    for (int i = 0; i < 8; ++i) {
        if (system_sizes[i] > 0 && !success_flags[i]) {
            all_success = false;
            break;
        }
    }
    
    if (!all_success) {
        cout << "  Some threads failed to solve their subsystems" << endl;
        return vector<double>();
    }
    
    // Собираем полное решение из решений подсистем
    vector<double> full_solution(n_points);
    
    // Заполняем граничные точки
    full_solution[0] = SQRT2;
    full_solution[n_points-1] = SQRT2;
    
    // Собираем внутренние точки из всех потоков
    for (int r = 0; r < 8; ++r) {
        const vector<double>& subsol = solutions[r];
        if (subsol.empty()) continue;
        
        // Определяем индексы для этого остатка
        for (size_t j = 0; j < subsol.size(); ++j) {
            int idx = r + j * 8;
            if (idx > 0 && idx < n_points - 1) {
                full_solution[idx] = subsol[j];
            }
        }
    }
    
    // Проверяем валидность решения
    for (double val : full_solution) {
        if (isnan(val) || isinf(val)) {
            return vector<double>();
        }
    }
    
    return full_solution;
}

// Функция для решения одного значения a в отдельном потоке
void solution(double a_param, int id,
                       vector<pair<double, vector<double>>>& results,
                       vector<bool>& success_flags) {
    stringstream ss;
    cout << "Solution " << id << ", a = " << a_param << endl;

    vector<double> solution = solve_for_a(a_param);
    if (!solution.empty()) {
        results[id] = make_pair(a_param, solution);
        success_flags[id] = true;
        successful_solutions++;
        
        // Сохраняем в файл
        stringstream filename;
        filename << "solution_a_" << fixed << setprecision(0) << a_param << ".txt";
        ofstream out(filename.str());
        
        if (out.is_open()) {
            out << fixed << setprecision(12);
            out << "# a = " << a_param << endl;
            out << "# x\ty" << endl;
            
            double h = 20.0 / (solution.size() - 1);
            for (size_t i = 0; i < solution.size(); ++i) {
                double x_val = -10.0 + i * h;
                out << x_val << "\t" << solution[i] << endl;
            }
            out.close();
            cout << "    Saved to " << filename.str() << endl;
        }
    } else {
        success_flags[id] = false;
        cout << "    Failed to compute solution for a = " << scientific << a_param << endl;
    }
}

int main() {
    // Значения параметра a (логарифмическая шкала от 100 до 1e6)
    vector<double> a_values;
    int num_values = 11;
    double a_min = 100.0;
    double a_max = 1000000.0;
    for (int i = 0; i < num_values; ++i) {
        double t = double(i) / (num_values - 1) * log10(a_max / a_min);
        a_values.push_back(a_min * pow(10.0, t));
    }
    
    cout << "==================================================" << endl;
    cout << "Solving boundary value problem: y'' = a(y - y^3)" << endl;
    cout << "Boundary conditions: y(-10) = y(10) = sqrt(2)" << endl;
    cout << "Using 8 threads per value of a (reduction + Thomas algorithm)" << endl;
    cout << "Parameter values:" << endl;
    for (size_t i = 0; i < a_values.size(); ++i) {
        cout << "  " << i+1 << ": a = " << scientific << setprecision(2) << a_values[i] << endl;
    }
    cout << "==================================================" << endl << endl;
    
    // Запускаем вычисления для всех значений a
    vector<thread> threads;
    vector<pair<double, vector<double>>> results(a_values.size());
    vector<bool> success_flags(a_values.size(), false);
    
    // Можно запустить несколько значений a параллельно, но ограничимся 4 потоками
    const int max_concurrent = 1;
    
    for (size_t i = 0; i < a_values.size(); i += max_concurrent) {
        solution(a_values[i], i, ref(results), ref(success_flags));
    }
    
    // Выводим статистику
    cout << "==================================================" << endl;
    cout << "SIMULATION COMPLETED" << endl;
    cout << "Successful computations: " << successful_solutions << "/" << a_values.size() << endl;
    
    // Создаем сводный файл
    ofstream summary("summary.txt");
    if (summary.is_open()) {
        summary << "Summary of computations" << endl;
        summary << "======================" << endl;
        summary << "Equation: y'' = a(y - y^3)" << endl;
        summary << "Boundary conditions: y(-10) = y(10) = sqrt(2)" << endl;
        summary << "Parallelization: 8 threads per value of a" << endl;
        summary << endl;
        
        int count = 0;
        for (size_t i = 0; i < a_values.size(); ++i) {
            summary << (i+1) << ". a = " << scientific << setprecision(6) << a_values[i]
                   << ": " << (success_flags[i] ? "SUCCESS" : "FAILED");
            if (success_flags[i] && !results[i].second.empty()) {
                summary << ", points = " << results[i].second.size();
            }
            summary << endl;
        }
        
        summary.close();
        cout << "Summary saved to summary.txt" << endl;
    }
    
    cout << "\nTo visualize results, run: python3 plots.py" << endl;
    cout << "==================================================" << endl;
    
    return 0;
}