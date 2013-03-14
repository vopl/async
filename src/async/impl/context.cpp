#include "async/impl/context.hpp"
#include "async/impl/scheduler.hpp"

namespace async { namespace impl
{

    Context::Context(Scheduler *scheduler, size_t stackSize)
        : _scheduler(scheduler)
    {
        _scheduler->contextCreate(&_state, stackSize);
    }

    Context::~Context()
    {
        _scheduler->contextDestroy(&_state);
    }

    bool Context::hasCode()
    {
        return _code.operator bool();
    }

    bool Context::setCode(const std::function<void()> &code)
    {
        if(!_code)
        {
            _code = code;
            return true;
        }

        return false;
    }

    void Context::activate()
    {
        assert(_code);

        _scheduler->markContextAsExec(this);

        _scheduler->contextActivate(&_state);

        if(_code)
        {
        	_scheduler->markContextAsHold(this);
        }
        else
        {
        	_scheduler->markContextAsEmpty(this);
        }
    }
}}
