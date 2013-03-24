#ifndef _ASYNC_WAIT_HPP_
#define _ASYNC_WAIT_HPP_

namespace async
{
    /*
     * event                wait
     * container<event>     wait
     * mutex                lock
     * container<mutex>     lock
     *
     */
    template <class... Waitable>
    size_t waitAny(const Waitable&... waitables);

    class Event;
    class Mutex;
}

namespace async { namespace impl
{
    class AnyWaiter;
    typedef std::shared_ptr<AnyWaiter> AnyWaiterPtr;

    AnyWaiterPtr waiterAlloc();
    void waiterPush(AnyWaiterPtr waiter, const ::async::Event &waitable);
    void waiterPush(AnyWaiterPtr waiter, const ::async::Mutex &waitable);
    size_t waiterExec(AnyWaiterPtr waiter);

    template <
            class Container,
            class = typename std::enable_if<
                std::is_same<
                    decltype(((const Container &)Container()).begin()),
                    decltype(((const Container &)Container()).end())
                >::value &&
                (
                    std::is_same<decltype(*((const Container &)Container()).begin()), const ::async::Event &>::value ||
                    std::is_same<decltype(*((const Container &)Container()).begin()), const ::async::Mutex &>::value ||
                    false
                ) &&
                true>::type
            >
    void waiterPush(AnyWaiterPtr waiter, const Container &container)
    {
        for(const auto &waitable : container)
        {
            waiterPush(waiter, waitable);
        }
    }

    inline void waiterCollect(AnyWaiterPtr waiter){}

    template <class FirstWaitable, class... Waitable>
    void waiterCollect(AnyWaiterPtr waiter, const FirstWaitable &firstWaitable, const Waitable&... waitables)
    {
        waiterPush(waiter, firstWaitable);
        waiterCollect(waiter, waitables...);
    }

}}

namespace async
{
    template <class... waitable>
    size_t waitAny(const waitable&... waitables)
    {
        impl::AnyWaiterPtr waiter = impl::waiterAlloc();
        impl::waiterCollect(waiter, waitables...);
        size_t res = impl::waiterExec(waiter);
        return res;
    }
}

#endif
