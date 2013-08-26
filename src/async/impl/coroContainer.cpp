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
        std::lock_guard<std::mutex> l(_mtx);

//    	if(!_hold.empty())
//    	{
//    		std::cerr<<_hold.size()<<std::endl;
//    	}
        assert(_ready.empty());
        //assert(_empty);
//        assert(_hold.empty());
//        assert(_exec.empty());
//        assert(_emitted.empty());
    }

    Coro *CoroContainer::te_emitWorkPiece()
    {
        std::lock_guard<std::mutex> l(_mtx);

    	if(_ready.empty())
    	{
    		return NULL;
    	}

        Coro *sp(_ready.front());
    	_ready.pop();

//    	assert(!_exec.count(sp));
//    	assert(!_hold.count(sp));
    	//assert(!_empty.count(sp));
    	//assert(!_ready.count(sp));
//    	assert(!_emitted.count(sp));

//    	_emitted.insert(sp);
        return sp;
    }

    void CoroContainer::spawn(const std::function<void()> &code)
    {
        std::lock_guard<std::mutex> l(_mtx);

        Coro *sp;
    	if(_empty.empty())
    	{
            sp = new Coro(static_cast<Scheduler *>(this));
    	}
    	else
    	{
    		sp = _empty.back();
    		_empty.pop_back();
    	}

    	sp->setCode(code);

        Scheduler *scheduler(static_cast<Scheduler*>(this));
        if(scheduler->pushWorkPiece(sp))
		{
//            _emitted.insert(sp);
            return;
		}
		else
		{
	    	_ready.push(sp);
		}
    }

    void CoroContainer::spawn(std::function<void()> &&code)
    {
        std::lock_guard<std::mutex> l(_mtx);

        Coro *sp;
        if(_empty.empty())
        {
            sp = new Coro(static_cast<Scheduler *>(this));
        }
        else
        {
            sp = _empty.back();
            _empty.pop_back();
        }

        sp->setCode(std::forward<std::function<void()>>(code));

        Scheduler *scheduler(static_cast<Scheduler*>(this));
        if(scheduler->pushWorkPiece(sp))
        {
//            _emitted.insert(sp);
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

        Coro *sp(coro);

        {
            std::lock_guard<std::mutex> l(_mtx);

//            assert(!_exec.count(sp));
//            assert(!_hold.count(sp));
            //assert(!_empty.count(sp));
            //assert(!_ready.count(sp));
//            assert(_emitted.count(sp));

//            _emitted.erase(sp);
//            _exec.insert(sp);
        }

        _current = coro;
    }

    void CoroContainer::markCoroHold(Coro *coro)
    {
        Coro *sp(coro);

        {
            std::lock_guard<std::mutex> l(_mtx);

//            assert(_exec.count(sp));
//            assert(!_hold.count(sp));
            //assert(!_empty.count(sp));
            //assert(!_ready.count(sp));
//            assert(!_emitted.count(sp));

//            _exec.erase(sp);

            assert(coro->hasCode());
//            _hold.insert(sp);
        }

        _current = nullptr;
    }

    void CoroContainer::markCoroComplete(Coro *coro)
    {
        Coro *sp(coro);

        {
            std::lock_guard<std::mutex> l(_mtx);

//            assert(_exec.count(sp));
//            assert(!_hold.count(sp));
            //assert(!_empty.count(sp));
            //assert(!_ready.count(sp));
//            assert(!_emitted.count(sp));

//            _exec.erase(sp);

            assert(!coro->hasCode());
            _empty.push_back(sp);
        }

        _current = nullptr;
    }

    Coro *CoroContainer::coroCurrent()
    {
        return _current;
    }

    void CoroContainer::coroReady(Coro *coro)
    {
        assert(coro->hasCode());

        Coro *sp(coro);
        Scheduler *scheduler = static_cast<Scheduler *>(this);

        {
            std::lock_guard<std::mutex> l(_mtx);

            if(scheduler->pushWorkPiece(coro))
            {
                return;
            }
            else
            {
                _ready.push(sp);
            }
        }
    }
}}
