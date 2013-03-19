#include "async/threadState.hpp"

namespace async
{
    ThreadState::ThreadState()
        : _value(init)
    {
    }

    ThreadState::~ThreadState()
    {
    }

    ThreadState::EValue ThreadState::get()
    {
        std::lock_guard<std::mutex> l(_mtx);
        return _value;
    }

    void ThreadState::wait(EValue v)
    {
        std::unique_lock<std::mutex> l(_mtx);
        while(_value != v)
        {
            _cv.wait(l);
        }
    }

    void ThreadState::waitNot(EValue v)
    {
        std::unique_lock<std::mutex> l(_mtx);
        while(_value == v)
        {
            _cv.wait(l);
        }
    }

    void ThreadState::set(EValue v)
    {
        std::lock_guard<std::mutex> l(_mtx);
        if(_value != v)
        {
            _value = v;
            _cv.notify_all();
        }
    }
}
