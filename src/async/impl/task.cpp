#include "async/impl/task.hpp"
#include "async/impl/scheduler.hpp"

namespace async { namespace impl
{

    Task::Task(Scheduler *scheduler, size_t stackSize)
        : _scheduler(scheduler)
    {
        _scheduler->contextCreate(this, stackSize);
    }

    Task::~Task()
    {
        _scheduler->contextDestroy(this);
    }

    bool Task::hasCode()
    {
        return _code.operator bool();
    }

    bool Task::setCode(const std::function<void()> &code)
    {
        if(!_code)
        {
            _code = code;
            return true;
        }

        return false;
    }

    void Task::activate()
    {
        assert(_code);

        _scheduler->markTaskAsExec(this);

        _scheduler->contextActivate(&_context);

        if(_code)
        {
            _scheduler->markTaskAsHold(this);
        }
        else
        {
            _scheduler->markTaskAsEmpty(this);
        }
    }

    void Task::contextProc()
    {
        assert(0);
    }

}}
