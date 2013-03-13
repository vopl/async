#include "async/impl/contextEngine.hpp"

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

	void ContextEngine::switchContextTo(Context *nextCtx)
	{
		//assert(current exists)
		//current=ctx
		//switch to ctx, return to previous current
		assert(0);
	}

}}
