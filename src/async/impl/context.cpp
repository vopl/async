#include "async/impl/context.hpp"
#include "async/impl/scheduler.hpp"

namespace async { namespace impl
{

    Context::Context(Scheduler *scheduler, size_t stackSize)
        : _scheduler(scheduler)
    {

    }

    Context::~Context()
    {

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

    Context::EActivationResult Context::activate()
    {
        if(!_code)
        {
            return ear_failNoCode;
        }

        _scheduler->resume(this);

        return _code?ear_okIncomplete:ear_okComplete;
    }
}}
