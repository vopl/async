#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"

namespace async { namespace impl
{

    Coro::Coro(Scheduler *scheduler, size_t stackSize)
        : _scheduler(scheduler)
    {
        _scheduler->contextCreate(this, stackSize);
    }

    Coro::~Coro()
    {
        _scheduler->contextDestroy(this);
    }

    bool Coro::hasCode()
    {
        return _code.operator bool();
    }

    void Coro::setCode(const std::function<void()> &code)
    {
    	assert(!_code);
		_code = code;
    }

    void Coro::activate()
    {
        assert(_code);

        _scheduler->markCoroAsExec(this);

        _scheduler->contextActivate(&_context);

        if(_code)
        {
            _scheduler->markCoroAsHold(this);
        }
        else
        {
            _scheduler->markCoroAsEmpty(this);
        }
    }

    void Coro::contextProc()
    {
        assert(0);
    }

}}
