#include "async/stable.hpp"
#include "async/impl/threadController.hpp"
#include "async/impl/scheduler.hpp"
#include <cassert>

namespace async { namespace impl
{
    ThreadController::ThreadController(Scheduler *scheduler)
        : _scheduler(scheduler)
        , _workPiece(NULL)
        , _releaseRequest(false)
    {
        if(!_scheduler->te_init(this))
        {
            _scheduler = NULL;
        }
    }

    ThreadController::~ThreadController()
    {
        if(_scheduler)
        {
            _scheduler->te_deinit();
        }
    }

    bool ThreadController::pushWorkPiece(const ContextPtr &workPiece)
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

    void ThreadController::pushReleaseRequest()
    {
        std::unique_lock<std::mutex> l(_mtx);
        _releaseRequest = true;
        _cv.notify_one();
    }

    void ThreadController::requestWorkPiece()
    {
        assert(!_workPiece);
        _workPiece = _scheduler->te_emitWorkPiece();
    }

}}
