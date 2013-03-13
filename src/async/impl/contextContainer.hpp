#ifndef _ASYNC_IMPL_CONTEXTCONTAINER_HPP_
#define _ASYNC_IMPL_CONTEXTCONTAINER_HPP_

#include "async/impl/context.hpp"

#include <mutex>
#include <queue>
#include <vector>
#include <set>

namespace async { namespace impl
{
    class ContextContainer
    {

    public:
        ContextContainer();
        ~ContextContainer();

    public://for thread
        Context* te_emitWorkPiece();

    public://for busines
        void pushCodeToRun(const std::function<void()> &code);

    public://for context
        void markContextAsExec(Context *ctx);
        void markContextAsHold(Context *ctx);
        void markContextAsEmpty(Context *ctx);

    private:

        std::mutex _mtx;
        std::queue<ContextPtr>	_ready;//fifo
        std::vector<ContextPtr>	_empty;//push popAny
        std::set<ContextPtr>	_hold;//insert search-pop
        std::set<ContextPtr>	_exec;//insert search-pop
        std::set<ContextPtr>	_emitted;//insert search-pop
    };
}}

#endif
