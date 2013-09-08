#ifndef _ASYNC_WAIT_HPP_
#define _ASYNC_WAIT_HPP_

#include "async/details/waiter.hpp"
#include <cstring>

namespace async
{
    //////////////////////////////////////////////////
    template <class... Waitable>
    uint32_t waitAny(Waitable &... waitables)
    {
        char synchronizerWaiterNodes[sizeof...(waitables) * sizeofImpl<impl::SynchronizerWaiterNode>::_value];
        memset(synchronizerWaiterNodes, 0, sizeof(synchronizerWaiterNodes));
        details::Waiter waiter(reinterpret_cast<impl::SynchronizerWaiterNode*>(synchronizerWaiterNodes));
        waiter.collectWaitables(waitables...);

        return waiter.any();
    }

}

#endif
