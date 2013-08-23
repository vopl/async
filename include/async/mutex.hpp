#ifndef _ASYNC_MUTEX_HPP_
#define _ASYNC_MUTEX_HPP_

#include "async/sizeofImpl.hpp"
#include "async/hiddenImpl.hpp"

namespace async
{
    class Mutex
            : public HiddenImpl<impl::Mutex>
    {
        using Base = HiddenImpl<impl::Mutex>;

    private:
        Mutex(const Mutex &other) = delete;
        Mutex &operator=(const Mutex &other) = delete;

    public:
        Mutex(bool recursive);
        ~Mutex();

        //void lock();
        bool tryLock();

        bool isLocked();
        void unlock();
    };
}

#endif
