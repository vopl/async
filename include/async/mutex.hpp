#ifndef _ASYNC_MUTEX_HPP_
#define _ASYNC_MUTEX_HPP_

#include "async/sizeofImpl.hpp"
#include "async/hiddenImpl.hpp"

namespace async
{
//    namespace impl
//    {
//        class Mutex;
//        class AnyWaiter;
//    }

    class Mutex
            : private HiddenImpl<impl::Mutex>
    {
        using Base = HiddenImpl<impl::Mutex>;

    public:
        Mutex(bool recursive);
        ~Mutex();

        //void lock();
        bool tryLock();

        bool isLocked();
        void unlock();

//    private:
//        friend class impl::AnyWaiter;
    };
}

#endif
