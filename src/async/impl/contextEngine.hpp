#ifndef _ASYNC_IMPL_CONTEXTENGINE_HPP_
#define _ASYNC_IMPL_CONTEXTENGINE_HPP_

#include "async/impl/thread.hpp"
#include "async/impl/context.hpp"

namespace async { namespace impl
{
    class ContextEngine
    {

    public:
        ContextEngine();
        ~ContextEngine();

    public://for thread
        bool te_init(Thread *thread);
        void te_deinit();

    public://for context
        void switchContextTo(Context *nextCtx);
    };
}}

#endif
