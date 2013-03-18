#include "async/stable.hpp"
#include "async/impl/coroContainer.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"

#include <cassert>

namespace async { namespace impl
{

    __thread Coro *CoroContainer::_current = nullptr;

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

        Scheduler *scheduler(static_cast<Scheduler*>(this));
        if(scheduler->pushWorkPiece(sp.get()))
		{
            _emitted.insert(sp);
            return;
		}
		else
		{
	    	_ready.push(sp);
		}
    }

    void CoroContainer::coroActivate(Coro *coro)
    {
        assert(coro->hasCode());

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

        //TODO here mistiming of context (in coro) and it`s state (in container)

        _current = coro;
        contextEngine->contextActivate(coro);
    }

    void CoroContainer::coroHold(Coro *coro, std::mutex &alienLockedForUnlock)
    {
        //assert(_mtx.is_locked());
        //assert(alienLockedForUnlock.is_locked());

        CoroPtr sp(coro->shared_from_this());
        Scheduler *scheduler = static_cast<Scheduler *>(this);
        ContextEngine *contextEngine = static_cast<ContextEngine *>(scheduler);

        {
            std::unique_lock<std::mutex> l(_mtx, std::adopt_lock);
            std::unique_lock<std::mutex> l2(alienLockedForUnlock, std::adopt_lock);

            assert(_exec.count(sp));
            assert(!_hold.count(sp));
            //assert(!_empty.count(sp));
            //assert(!_ready.count(sp));
            assert(!_emitted.count(sp));

            _exec.erase(sp);

            assert(coro->hasCode());
            _hold.insert(sp);
        }

        _current = nullptr;
        contextEngine->contextDeactivate(coro);
    }

    Coro *CoroContainer::coroCurrent()
    {
        return _current;
    }

    std::mutex &CoroContainer::mtxForCoroHold()
    {
        return _mtx;
    }

    void CoroContainer::coroReadyIfHolded(Coro *coro)
    {
        assert(coro->hasCode());

        CoroPtr sp(coro->shared_from_this());
        Scheduler *scheduler = static_cast<Scheduler *>(this);
        ContextEngine *contextEngine = static_cast<ContextEngine *>(scheduler);

        {
            std::unique_lock<std::mutex> l(_mtx);

            std::set<CoroPtr>::iterator holdIter = _hold.find(sp);
            if(_hold.end() == holdIter)
            {
                char tmp[32];
                sprintf(tmp, "check       %p\n", coro);
                std::cout<<tmp; std::cout.flush();
                //assert(!"check this");
                //already execute
                return;
            }

            _hold.erase(holdIter);

            assert(!_exec.count(sp));
            assert(!_hold.count(sp));
            //assert(!_empty.count(sp));
            //assert(!_ready.count(sp));
            assert(!_emitted.count(sp));

            if(scheduler->pushWorkPiece(coro))
            {
                _emitted.insert(sp);
                return;
            }
            else
            {
                _ready.push(sp);
            }
        }
    }

    void CoroContainer::coroComplete(Coro *coro)
    {
        CoroPtr sp(coro->shared_from_this());
        Scheduler *scheduler = static_cast<Scheduler *>(this);
        ContextEngine *contextEngine = static_cast<ContextEngine *>(scheduler);

        {
            std::unique_lock<std::mutex> l(_mtx);

            assert(_exec.count(sp));
            assert(!_hold.count(sp));
            //assert(!_empty.count(sp));
            //assert(!_ready.count(sp));
            assert(!_emitted.count(sp));

            _exec.erase(sp);

            assert(!coro->hasCode());
            _empty.push_back(sp);
        }

        _current = nullptr;
        contextEngine->contextDeactivate(coro);
    }


}}
