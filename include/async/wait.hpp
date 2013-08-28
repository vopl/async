#ifndef _ASYNC_WAIT_HPP_
#define _ASYNC_WAIT_HPP_

#include "async/details/waiter.hpp"

namespace async
{
    //////////////////////////////////////////////////
    template <class... Waitable>
    uint32_t waitAny(Waitable &... waitables)
    {
        impl::Synchronizer *synchronizersBuffer[sizeof...(waitables)];
        details::Waiter waiter(synchronizersBuffer);
        waiter.collectWaitables(waitables...);

        return waiter.any();
    }

}

#endif
