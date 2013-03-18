#ifndef _ASYNC_IMPL_THREAD_HPP_
#define _ASYNC_IMPL_THREAD_HPP_

#include "async/threadUtilizer.hpp"
#include "async/impl/scheduler.hpp"

#include <mutex>
#include <condition_variable>
#include <cassert>


namespace async { namespace impl
{
    class Scheduler;
    class Coro;

    class Thread
    {
    public:
        Thread(Scheduler *scheduler, ThreadState *stateEvt);
        ~Thread();

        template <class LimitCounter>
        EThreadUtilizationResult utilize(LimitCounter &limitCounter);

    public:
        bool pushWorkPiece(Coro *workPiece);
        void pushReleaseRequest();

    private:
        void requestWorkPiece();

    private:
        Scheduler *_scheduler;
        ThreadState *_stateEvt;

        std::mutex _mtx;
        std::condition_variable _cv;

        Coro* _workPiece;
        bool _releaseRequest;
    };

    template <class LimitCounter>
    EThreadUtilizationResult Thread::utilize(LimitCounter &limitCounter)
    {
        if(_stateEvt)
        {
            _stateEvt->set(ThreadState::inWork);
        }

        if(!_scheduler)
        {
            return etur_notBeenUtilized;
        }

        std::unique_lock<std::mutex> lock(_mtx);

        for(;;)
        {
            if(!_workPiece)
            {
                requestWorkPiece();

                if(!_workPiece)
                {
                    limitCounter.wait(_cv, lock);
                }
            }

            while(_workPiece)
            {
                Coro* workPiece(NULL);
                std::swap(workPiece, _workPiece);
                lock.unlock();

                //do work
                {
                    _scheduler->coroActivate(workPiece);
                }

                lock.lock();
                limitCounter.incrementWorkAmount();
            }

            if(_releaseRequest)
            {
                assert(!_workPiece);

                if(_stateEvt)
                {
                    _stateEvt->set(ThreadState::doneWork);
                }

                return etur_releaseRequest;
            }

            if(limitCounter.completed())
            {
                assert(!_workPiece);

                if(_stateEvt)
                {
                    _stateEvt->set(ThreadState::doneWork);
                }

                return etur_limitExhausted;
            }
        }
    }

}}

#endif
