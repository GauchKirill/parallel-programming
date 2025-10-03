#ifndef INTEGR_STACK_H
#define INTEGR_STACK_H

#include <stack>
#include <cstdint>

namespace integr
{
namespace Size
{
    const size_t GLOBAL   = 5000U;
    const size_t LOCAL    = 200U;

    const size_t LOAD     = 5U; 
    const size_t DROP     = 20U;
}; // namespace Size

class Task
{
  public:
    double l;
    double r;
    double f_l;
    double f_r;
    double s;
  
  public:
    Task() = default;
    Task(   double left,
            double right,
            double f_left,
            double f_right,
            double square) :
            l(left),
            r(right),
            f_l(f_left),
            f_r(f_right),
            s(square) {}

    ~Task() = default;
};

template<size_t STACK_SIZE>
class Stack
{
  private:
    Task tasks[STACK_SIZE];
    std::size_t sp = 0;

  public:
    Stack() = default;

    size_t capasity(void) const;
    void put(Task&& rhs);
    Task&& pop();
    std::size_t get_sp(void) const;

    ~Stack() = default;

}; // class Stack


template<size_t STACK_SIZE> 
void Stack<STACK_SIZE>::put(Task&& rhs)
{
    if (sp < STACK_SIZE)
        tasks[sp++] = std::move(rhs);
}

template<size_t STACK_SIZE>
Task&& Stack<STACK_SIZE>::pop()
{
    if (sp > 0)
        return std::move(tasks[--sp]);
    
    return std::move(tasks[0]);
}

template<size_t STACK_SIZE>
size_t Stack<STACK_SIZE>::capasity(void) const
{
    return STACK_SIZE;
}

template<size_t STACK_SIZE>
size_t Stack<STACK_SIZE>::get_sp(void) const
{
    return sp;
}

} // namespace integr

#endif