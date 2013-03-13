#include "async/stable.hpp"
#include "async/impl/scheduler.hpp"
#include <cassert>

namespace async { namespace impl
{
    Scheduler::Scheduler()
    {
    }

    Scheduler::~Scheduler()
    {
    }

    bool Scheduler::te_init(Thread *thread)
    {
    	if(!ThreadContainer::te_init(thread))
    	{
    		//TODO log reason
    		return false;
    	}
    	if(!ContextEngine::te_init(thread))
    	{
    		//TODO log reason
    		ThreadContainer::te_deinit();
    		return false;
    	}

    	return true;
    }

    void Scheduler::te_deinit()
    {
    	ContextEngine::te_deinit();
    	ThreadContainer::te_deinit();
    }

}}
