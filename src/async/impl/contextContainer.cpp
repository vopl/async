#include "async/impl/contextContainer.hpp"
#include "async/impl/scheduler.hpp"
#include <atomic>

namespace async { namespace impl
{

    ContextContainer::ContextContainer()
    {

    }

    ContextContainer::~ContextContainer()
    {

    }

    extern std::atomic<size_t> g_counter;
    ContextPtr ContextContainer::te_emitWorkPiece()
    {
        ContextPtr ctx(new Context(static_cast<Scheduler*>(this)));
        ctx->setCode([]{
            volatile int c(0);
            for(int k(0); k<10; k++)
            {
                c+=k;
            }
            async::impl::g_counter++;
        });
        return ctx;
    }

    void ContextContainer::suspend(Context *ctx)
    {
        assert(0);
    }

    void ContextContainer::resume(Context *ctx)
    {
        assert(0);
    }


}}
