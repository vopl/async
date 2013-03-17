#include "async/stable.hpp"
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
        _scheduler->coroCodeExecute(this);
    }

    void Coro::contextProc()
    {
        for(;;)
        {
            assert(_code);

            try
            {
                _code();
            }
            catch(const std::exception &e)
            {
                std::cerr<<__FUNCTION__<<", std exception catched: "<<e.what();
            }
            catch(...)
            {
                std::cerr<<__FUNCTION__<<", unknown exception catched";
            }

            assert(_code);
            std::function<void()>().swap(_code);

            _scheduler->coroCodeExecuted(this);
        }
    }

}}
