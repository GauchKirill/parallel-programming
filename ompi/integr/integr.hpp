#ifndef INTEGR_H
#define INTEGR_H

#include <mutex>
#include <semaphore.h>
#include <thread>
#include <functional>

namespace integr
{
class Integr
{
  private:
    const double left_;
    const double right_;
    const double eps_;
    std::function<double(double)> func_;

    const size_t start_num_tasks = 4U;

    double sum = 0.0f;
    size_t n_active = 0U;
    size_t num_threads_ = std::thread::hardware_concurrency();

    // Mutex доступа к стеку
    std::mutex m_stack;
    // Mutex доступа к глобальной сумме
    std::mutex m_sum;
    // Mutex наличия задач в глобальном стеке
    std::mutex m_task_present;

  public:
    Integr() = delete;
    Integr( double left,
            double right,
            double eps,
            std::function<double(double)> func);

    void set_num_threads(size_t num_threads);
    size_t get_num_threads(void) const;
    double get_result(void);
    ~Integr() = default;

  private:
    void integr_thread(void);
}; // class Integr

} // namespace integr
#endif