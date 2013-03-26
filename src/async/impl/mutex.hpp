#ifndef _ASYNC_IMPL_MUTEX_HPP_
#define _ASYNC_IMPL_MUTEX_HPP_

#include "async/impl/synchronizer.hpp"

namespace async { namespace impl
{
    class Mutex
            : public Synchronizer
    {
    private:
        Mutex(const Mutex &other);//not impl
        Mutex &operator=(const Mutex &other);//not impl

    public:
        Mutex(bool recursive);
        virtual ~Mutex();

        void lock();
        bool tryLock();

        bool isLocked();
        void unlock();

    private:
        virtual bool waiterAdd(AnyWaiterPtr waiter);

    private:
        bool _recursive;
        CoroPtr _owner;
        size_t _ownerDepth;
    };
}}

#endif
