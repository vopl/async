#include "async/stable.hpp"
#include "async/impl/thread.hpp"
#include "async/impl/scheduler.hpp"
#include <cassert>

namespace async { namespace impl
{
    Thread::Thread(Scheduler *scheduler)
        : _scheduler(scheduler)
        , _workPiece(NULL)
        , _releaseRequest(false)
    {
        if(!_scheduler->te_init(this))
        {
            _scheduler = NULL;
        }
    }

    Thread::~Thread()
    {
        if(_scheduler)
        {
            _scheduler->te_deinit();
        }
    }

    bool Thread::pushWorkPiece(Context *workPiece)
    {
        std::unique_lock<std::mutex> l(_mtx, std::defer_lock);

        if(l.try_lock())
        {
            assert(!_workPiece);
            if(!_workPiece)
            {
                _workPiece = workPiece;
                _cv.notify_one();
                return true;
            }
        }
        return false;
    }

    void Thread::pushReleaseRequest()
    {
        std::unique_lock<std::mutex> l(_mtx);
        _releaseRequest = true;
        _cv.notify_one();
    }

    void Thread::requestWorkPiece()
    {
        assert(!_workPiece);
        _workPiece = _scheduler->te_emitWorkPiece();
    }

}}
