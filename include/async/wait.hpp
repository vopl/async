#ifndef _ASYNC_WAIT_HPP_
#define _ASYNC_WAIT_HPP_

#include "async/details/multiWaiter.hpp"

namespace async
{
    //////////////////////////////////////////////////
    template <class... Waitable>
    uint32_t waitAny(Waitable &... waitables)
    {
        impl::Synchronizer *synchronizersBuffer[sizeof...(waitables)];
        details::MultiWaiter multiWaiter(synchronizersBuffer);
        multiWaiter.collectWaitables(waitables...);

        return multiWaiter.waitAny();
    }

}

#endif
