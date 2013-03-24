#ifndef _ASYNC_IMPL_ANYWAITER_HPP_
#define _ASYNC_IMPL_ANYWAITER_HPP_

#include <memory>
#include <mutex>
#include <deque>

namespace async
{
    class Event;
    class Mutex;
}

namespace async { namespace impl
{
    class Synchronizer;
    typedef std::shared_ptr<Synchronizer> SynchronizerPtr;

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

    protected:
        //from Synchronizer
        friend class ::async::impl::Synchronizer;
        bool notify(Synchronizer *notifier);

    private:
        std::mutex  _mtx;
        std::deque<SynchronizerPtr> _synchronizers;
        size_t _notified;
        CoroPtr _coro;
    };

}}

#endif
