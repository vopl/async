#ifndef _ASYNC_DETAILS_MULTIWAITER_HPP_
#define _ASYNC_DETAILS_MULTIWAITER_HPP_

#include "async/sizeofImpl.hpp"
#include "async/hiddenImpl.hpp"

#include <cstdint>

namespace async
{
    class Mutex;
    class Event;

    namespace impl
    {
        class Synchronizer;
    }

    namespace details
    {
        class MultiWaiter
                : private HiddenImpl<impl::MultiWaiter>
        {
            using Base = HiddenImpl<impl::MultiWaiter>;

        private:
            MultiWaiter(const MultiWaiter &other) = delete;
            MultiWaiter &operator=(const MultiWaiter &other) = delete;

        public:
            MultiWaiter(impl::Synchronizer **synchronizersBuffer);
            ~MultiWaiter();

            uint32_t waitAny();
            void waitAll();

            template <class... Waitable>
            void collectWaitables(Mutex &mutex, Waitable &... waitables);

            template <class... Waitable>
            void collectWaitables(Event &event, Waitable &... waitables);

            void collectWaitables();

        private:
            void push(Mutex &mutex);
            void push(Event &event);

        };


        //////////////////////////////////////////////////
        inline void MultiWaiter::collectWaitables()
        {
            //empty ok
        }

        //////////////////////////////////////////////////
        template <class... Waitable>
        void MultiWaiter::collectWaitables(Mutex &mutex, Waitable&... waitables)
        {
            push(mutex);
            collectWaitables(waitables...);
        }

        //////////////////////////////////////////////////
        template <class... Waitable>
        void MultiWaiter::collectWaitables(Event &event, Waitable&... waitables)
        {
            push(event);
            collectWaitables(waitables...);
        }
    }
}

#endif
