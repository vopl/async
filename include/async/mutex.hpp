#ifndef _ASYNC_MUTEX_HPP_
#define _ASYNC_MUTEX_HPP_

#include <memory>

namespace async
{
    namespace impl
    {
        class Mutex;
        typedef std::shared_ptr<Mutex> MutexPtr;

        class AnyWaiter;
    }

    class Mutex
    {
    public:
        Mutex(bool recursive);
        Mutex(const Mutex &other);
        Mutex(Mutex &&other);
        ~Mutex();

        Mutex &operator=(const Mutex &other);
        Mutex &operator=(Mutex &&other);

        void swap(Mutex &other);

        //void lock();
        bool tryLock();

        bool isLocked();
        void unlock();

    private:
        friend class impl::AnyWaiter;
        impl::MutexPtr _implMutex;
    };
}

#endif
