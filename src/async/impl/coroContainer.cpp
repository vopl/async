#include "async/impl/coroContainer.hpp"
#include "async/impl/scheduler.hpp"
#include <atomic>

namespace async { namespace impl
{

    CoroContainer::CoroContainer()
    {

    }

    CoroContainer::~CoroContainer()
    {
    	std::unique_lock<std::mutex> l(_mtx);

        assert(_ready.empty());
        //assert(_empty);
        assert(_hold.empty());
        assert(_exec.empty());
        assert(_emitted.empty());
    }

    Coro *CoroContainer::te_emitWorkPiece()
    {
    	std::unique_lock<std::mutex> l(_mtx);

    	if(_ready.empty())
    	{
    		return NULL;
    	}

        CoroPtr sp(_ready.front());
    	_ready.pop();

    	assert(!_exec.count(sp));
    	assert(!_hold.count(sp));
    	//assert(!_empty.count(sp));
    	//assert(!_ready.count(sp));
    	assert(!_emitted.count(sp));

    	_emitted.insert(sp);
    	return sp.get();
    }

    void CoroContainer::spawn(const std::function<void()> &code)
    {
    	std::unique_lock<std::mutex> l(_mtx);

        CoroPtr sp;
    	if(_empty.empty())
    	{
            sp.reset(new Coro(static_cast<Scheduler *>(this)));
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
            _emitted.insert(sp);
            return;
		}
		else
		{
	    	_ready.push(sp);
		}
    }

    void CoroContainer::coroCodeExecute(Coro *coro)
    {
        CoroPtr sp(coro->shared_from_this());
        Scheduler *scheduler = static_cast<Scheduler *>(this);
        ContextEngine *contextEngine = static_cast<ContextEngine *>(scheduler);

        {
            std::unique_lock<std::mutex> l(_mtx);

            assert(!_exec.count(sp));
            assert(!_hold.count(sp));
            //assert(!_empty.count(sp));
            //assert(!_ready.count(sp));
            assert(_emitted.count(sp));

            _emitted.erase(sp);
            _exec.insert(sp);
        }

        contextEngine->contextActivate(coro);

        {
            std::unique_lock<std::mutex> l(_mtx);

            assert(_exec.count(sp));
            assert(!_hold.count(sp));
            //assert(!_empty.count(sp));
            //assert(!_ready.count(sp));
            assert(!_emitted.count(sp));

            _exec.erase(sp);

            if(coro->hasCode())
            {
                _hold.insert(sp);
            }
            else
            {
                _empty.push_back(sp);
            }
        }
    }

    void CoroContainer::coroCodeExecuted(Coro *coro)
    {
        Scheduler *scheduler = static_cast<Scheduler *>(this);
        ContextEngine *contextEngine = static_cast<ContextEngine *>(scheduler);

        contextEngine->contextDeactivate(coro);
    }

}}
