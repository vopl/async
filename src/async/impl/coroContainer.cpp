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

    	if(!_hold.empty())
    	{
    		std::cerr<<_hold.size()<<std::endl;
    	}
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

    void CoroContainer::markCoroExec(Coro *coro)
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

        _current = coro;
    }

    void CoroContainer::markCoroHold(Coro *coro)
    {
        CoroPtr sp(coro->shared_from_this());
        Scheduler *scheduler = static_cast<Scheduler *>(this);
        ContextEngine *contextEngine = static_cast<ContextEngine *>(scheduler);

        {
            std::lock_guard<std::mutex> l(_mtx);

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
    }

    void CoroContainer::markCoroComplete(Coro *coro)
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
    }

    Coro *CoroContainer::coroCurrent()
    {
        return _current;
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
                assert(!"check this, secondary context wakeup after multi wait");
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
}}
