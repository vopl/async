#ifndef _ASYNC_IMPL_ANYWAITER_HPP_
#define _ASYNC_IMPL_ANYWAITER_HPP_

#include <memory>
#include <mutex>
#include <deque>
#include <atomic>

namespace async
{
    class Event;
    class Mutex;
}

namespace async { namespace impl
{
    class Synchronizer;
    typedef std::shared_ptr<Synchronizer> SynchronizerPtr;
    class Mutex;
    class Event;

    class Coro;
    typedef std::shared_ptr<Coro> CoroPtr;

    class AnyWaiter
            : public std::enable_shared_from_this<AnyWaiter>
    {
    public:
        AnyWaiter();
        ~AnyWaiter();

        void push(const ::async::Event &waitable);
        void push(const ::async::Mutex &waitable);

        size_t exec();

        const CoroPtr &getCoro();

    protected:
        //from Synchronizer
        friend class ::async::impl::Synchronizer;
        friend class ::async::impl::Event;
        friend class ::async::impl::Mutex;
        bool notify(Synchronizer *notifier);

    private:
        //std::mutex  _mtx;
        std::deque<SynchronizerPtr> _synchronizers;
        std::deque<SynchronizerPtr> _synchronizersInitial;
        std::atomic<size_t> _notified;
        static const size_t markActive = (size_t)-1;
        static const size_t markPreNotified = (size_t)-2;
        static const size_t markDeactivating = (size_t)-3;
        CoroPtr _coro;
    };

}}

#endif
