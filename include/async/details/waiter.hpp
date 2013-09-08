#ifndef _ASYNC_DETAILS_WAITER_HPP_
#define _ASYNC_DETAILS_WAITER_HPP_

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
        class Waiter
                : private HiddenImpl<impl::Waiter>
        {
            using Base = HiddenImpl<impl::Waiter>;

        private:
            Waiter(const Waiter &other) = delete;
            Waiter &operator=(const Waiter &other) = delete;

        public:
            Waiter(impl::SynchronizerWaiterNode *synchronizerWaiterNodes);
            ~Waiter();

            uint32_t any();
            void all();

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
        inline void Waiter::collectWaitables()
        {
            //empty ok
        }

        //////////////////////////////////////////////////
        template <class... Waitable>
        void Waiter::collectWaitables(Mutex &mutex, Waitable&... waitables)
        {
            push(mutex);
            collectWaitables(waitables...);
        }

        //////////////////////////////////////////////////
        template <class... Waitable>
        void Waiter::collectWaitables(Event &event, Waitable&... waitables)
        {
            push(event);
            collectWaitables(waitables...);
        }
    }
}

#endif
