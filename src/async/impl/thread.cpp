#include "async/stable.hpp"
#include "async/impl/thread.hpp"
#include "async/impl/scheduler.hpp"
#include <cassert>

namespace async { namespace impl
{
    Thread::Thread(Scheduler *scheduler, ThreadState *stateEvt)
        : _scheduler(scheduler)
        , _stateEvt(stateEvt)
        , _workPiece(NULL)
        , _releaseRequest(false)
    {
        if(_stateEvt)
        {
            _stateEvt->set(ThreadState::init);
        }
        if(!_scheduler->te_init(this))
        {
            _scheduler = NULL;
        }
        else
        {
            if(_stateEvt)
            {
                _stateEvt->set(ThreadState::inited);
            }
        }
    }

    Thread::~Thread()
    {
        if(_stateEvt)
        {
            _stateEvt->set(ThreadState::deinit);
        }
        if(_scheduler)
        {
            _scheduler->te_deinit();
        }
        if(_stateEvt)
        {
            _stateEvt->set(ThreadState::deinited);
        }

    }

    bool Thread::pushWorkPiece(Coro *workPiece)
    {
        std::unique_lock<std::mutex> l(_mtx, std::defer_lock);

        if(l.try_lock())
        {
            if(!_releaseRequest && !_workPiece)
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
        std::lock_guard<std::mutex> l(_mtx);
        _releaseRequest = true;
        _cv.notify_one();
    }

    void Thread::requestWorkPiece()
    {
        assert(!_workPiece);
        _workPiece = _scheduler->te_emitWorkPiece();
    }

}}
