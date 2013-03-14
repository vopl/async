#include "async/impl/contextEngine.hpp"
#include "async/impl/context.hpp"
#include "async/impl/thread.hpp"

#include <cassert>

namespace async { namespace impl
{
    ContextEngine::ContextEngine()
	{
	}

	ContextEngine::~ContextEngine()
	{
	}

	bool ContextEngine::te_init(Thread *thread)
	{
		//create root context
		assert(0);
		return false;
	}

	void ContextEngine::te_deinit()
	{
		//destroy root context
		assert(0);
	}

    void ContextEngine::contextCreate(ContextState *ctx, size_t stackSize)
    {
        assert(0);
    }

    void ContextEngine::contextActivate(ContextState *ctx)
	{
		//assert(current exists)
		//current=ctx
		//switch to ctx, return to previous current
		assert(0);
	}

    void ContextEngine::contextDestroy(ContextState *ctx)
    {
        assert(0);
    }


}}
