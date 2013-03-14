#include "async/impl/taskContainer.hpp"
#include "async/impl/scheduler.hpp"
#include <atomic>

namespace async { namespace impl
{

    TaskContainer::TaskContainer()
    {

    }

    TaskContainer::~TaskContainer()
    {
    	std::unique_lock<std::mutex> l(_mtx);

        assert(_ready.empty());
        //assert(_empty);
        assert(_hold.empty());
        assert(_exec.empty());
        assert(_emitted.empty());
    }

    Task *TaskContainer::te_emitWorkPiece()
    {
    	std::unique_lock<std::mutex> l(_mtx);

    	if(_ready.empty())
    	{
    		return NULL;
    	}

        TaskPtr sp(_ready.front());
    	_ready.pop();

    	assert(!_exec.count(sp));
    	assert(!_hold.count(sp));
    	//assert(!_empty.count(sp));
    	//assert(!_ready.count(sp));
    	assert(!_emitted.count(sp));

    	_emitted.insert(sp);
    	return sp.get();
    }

    void TaskContainer::pushCodeToRun(const std::function<void()> &code)
    {
    	std::unique_lock<std::mutex> l(_mtx);

        TaskPtr sp;
    	if(_empty.empty())
    	{
            sp.reset(new Task(static_cast<Scheduler *>(this)));
    	}
    	else
    	{
    		sp = _empty.back();
    		_empty.pop_back();
    	}

    	sp->setCode(code);

		Scheduler *sched(static_cast<Scheduler*>(this));
		if(sched->pushWorkPiece(sp.get()))
		{
			return;
		}
		else
		{
	    	_ready.push(sp);
		}
    }

    void TaskContainer::markTaskAsExec(Task *ctx)
    {
        TaskPtr sp(ctx->shared_from_this());

    	std::unique_lock<std::mutex> l(_mtx);

    	assert(!_exec.count(sp));
    	assert(!_hold.count(sp));
    	//assert(!_empty.count(sp));
    	//assert(!_ready.count(sp));
    	assert(_emitted.count(sp));

    	_emitted.erase(sp);
    	_exec.insert(sp);
    }

    void TaskContainer::markTaskAsHold(Task *ctx)
    {
        TaskPtr sp(ctx->shared_from_this());

    	std::unique_lock<std::mutex> l(_mtx);

    	assert(_exec.count(sp));
    	assert(!_hold.count(sp));
    	//assert(!_empty.count(sp));
    	//assert(!_ready.count(sp));
    	assert(!_emitted.count(sp));

    	_exec.erase(sp);
    	_hold.insert(sp);
    }

    void TaskContainer::markTaskAsEmpty(Task *ctx)
    {
        TaskPtr sp(ctx->shared_from_this());

    	std::unique_lock<std::mutex> l(_mtx);

    	assert(_exec.count(sp));
    	assert(!_hold.count(sp));
    	//assert(!_empty.count(sp));
    	//assert(!_ready.count(sp));
    	assert(!_emitted.count(sp));

    	_exec.erase(sp);
    	_empty.push_back(sp);
    }


}}
