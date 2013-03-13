#ifndef _ASYNC_IMPL_CONTEXTCONTAINER_HPP_
#define _ASYNC_IMPL_CONTEXTCONTAINER_HPP_

#include "async/impl/context.hpp"

namespace async { namespace impl
{
    class ContextContainer
    {

    public:
        ContextContainer();
        ~ContextContainer();

    public://for thread
        ContextPtr te_emitWorkPiece();

    public://for busines
        void pushCode(const std::function<void()> &code);

    public:
        void suspend(Context *ctx);
        void resume(Context *ctx);
    };
}}

#endif
