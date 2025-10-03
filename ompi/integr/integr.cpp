#include <vector>
#include <chrono>
#include <random>
#include "integr.hpp"
#include "integr_stack.hpp"

namespace integr
{

Stack<Size::GLOBAL> global_stack;

Integr::Integr (
    double left,
    double right,
    double eps,
    std::function<double(double)> func) :

    left_(left),
    right_(right),
    eps_(eps),
    func_(func) {}

void Integr::set_num_threads(size_t num_threads)
{
    num_threads_ = num_threads;
}

size_t Integr::get_num_threads(void) const
{
    return num_threads_;
}

double Integr::get_result (void)
{
    std::vector<std::thread*> threads;
    n_active = num_threads_;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> dist(left_, right_);

    std::vector<double> arr(start_num_tasks);
    for (size_t i = 0; i < start_num_tasks; ++i)
        arr[i] = dist(gen);

    std::sort(arr.begin(), arr.end());

    double f_l = func_(left_);
    double f_r = func_(arr[0]);
    global_stack.put(Task(left_, arr[0], f_l, f_r, (f_l + f_r)/2));

    for (size_t i = 0; i < start_num_tasks-1; ++i)
    {
        f_l = func_(arr[i]);
        f_r = func_(arr[i+1]);
        global_stack.put(Task(arr[i], arr[i+1], f_l, f_r, (f_l + f_r)/2));
    }

    f_l = func_(arr[start_num_tasks-1]);
    f_r = func_(right_);
    global_stack.put(Task(arr[start_num_tasks-1], right_, f_l, f_r, (f_l + f_r)/2));

    for (uint8_t i = 1; i < num_threads_; ++i)
        threads.push_back(new std::thread(&Integr::integr_thread, this));

    integr_thread();

    for (std::thread* thr : threads)
    {
        thr->join();
        delete thr;
    }
    
    threads.clear();
    return sum;
}

void Integr::integr_thread (void)
{
    Stack<Size::LOCAL> local_stack;
    double local_sum = 0.0f;
    
    while (true)
    {
        if (local_stack.get_sp() == 0)
        {
            m_stack.lock();
            --n_active;
            if (!n_active && !global_stack.get_sp())
            {
                m_stack.unlock();
                m_task_present.unlock();
                break;
            }
            m_stack.unlock();

            m_task_present.lock();

            m_stack.lock();
            size_t load = std::min<size_t>(global_stack.get_sp(), Size::LOAD);
            if (!load)
            {
                ++n_active;
                m_stack.unlock();
            }
            else
            {
                for (size_t i = 0; i < load; ++i)
                    local_stack.put(global_stack.pop());
            
                ++n_active;
                size_t sp = global_stack.get_sp();
                m_stack.unlock();

                if (sp) m_task_present.unlock(); 
            }

        }
        else
        if (local_stack.get_sp() == local_stack.capasity())
        {
            m_stack.lock();
            size_t sp = global_stack.get_sp();
            for (size_t i = 0; i < Size::DROP; ++i)
                global_stack.put(local_stack.pop());
            
            if (!sp) m_task_present.unlock();

            m_stack.unlock();
        }
        else
        {
            Task task = local_stack.pop();
            double m = (task.l + task.r)/2;
            double f_m = func_(m);

            double s_l = (m - task.l)*(task.f_l + f_m)/2;
            double s_r = (task.r - m)*(task.f_r + f_m)/2;

            double eps = eps_*(task.r - task.l)/(right_ - left_)/2;

            if ((s_l + s_r - task.s > -eps) &&
                (s_l + s_r - task.s < eps))
                local_sum += s_l + s_r;
            
            else
            {
                local_stack.put(Task(task.l, m, task.f_l, f_m, s_l));
                local_stack.put(Task(m, task.r, f_m, task.f_r, s_r));
            }
        }
    }

    m_sum.lock();
    sum += local_sum;
    m_sum.unlock();

}

} // namespace integr