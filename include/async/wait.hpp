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
    size_t wait(const Waitable&... waitables);

    class Event;
    class Mutex;
}

namespace async { namespace impl
{
    class Waiter;

    Waiter *waiterAlloc();
    void waiterPush(Waiter *waiter, const ::async::Event &waitable);
    void waiterPush(Waiter *waiter, const ::async::Mutex &waitable);
    size_t waiterExec(Waiter *waiter);

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
    void waiterPush(Waiter *waiter, const Container &container)
    {
        for(const auto &waitable : container)
        {
            waiterPush(waiter, waitable);
        }
    }

    inline void waiterCollect(Waiter *waiter){}

    template <class FirstWaitable, class... Waitable>
    void waiterCollect(Waiter *waiter, const FirstWaitable &firstWaitable, const Waitable&... waitables)
    {
        waiterPush(waiter, firstWaitable);
        waiterCollect(waiter, waitables...);
    }

}}

namespace async
{
    template <class... waitable>
    size_t wait(const waitable&... waitables)
    {
        impl::Waiter *waiter = impl::waiterAlloc();
        impl::waiterCollect(waiter, waitables...);
        size_t res = impl::waiterExec(waiter);
        return res;
    }
}

#endif
