#ifndef _ASYNC_IMPL_THREADCONTROLLER_HPP_
#define _ASYNC_IMPL_THREADCONTROLLER_HPP_

#include "async/threadUtilizer.hpp"
#include "async/impl/context.hpp"

#include <mutex>
#include <condition_variable>
#include <cassert>


namespace async { namespace impl
{
    class Scheduler;

    class ThreadController
    {
    public:
        ThreadController(Scheduler *scheduler);
        ~ThreadController();

        template <class LimitCounter>
        EThreadUtilizationResult utilize(LimitCounter &limitCounter);

    public:
        bool pushWorkPiece(const ContextPtr &workPiece);
        void pushReleaseRequest();

    private:
        void requestWorkPiece();

    private:
        Scheduler *_scheduler;

        std::mutex _mtx;
        std::condition_variable _cv;

        ContextPtr _workPiece;
        bool _releaseRequest;
    };

    template <class LimitCounter>
    EThreadUtilizationResult ThreadController::utilize(LimitCounter &limitCounter)
    {
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

            if(_workPiece)
            {
                ContextPtr workPiece;
                workPiece.swap(_workPiece);
                lock.unlock();

                //do work
                {
                    workPiece->activate();
                }

                lock.lock();
                limitCounter.incrementWorkAmount();
            }

            if(_releaseRequest)
            {
                assert(!_workPiece);
                return etur_releaseRequest;
            }

            if(limitCounter.completed())
            {
                assert(!_workPiece);
                return etur_limitExhausted;
            }
        }
    }

}}

#endif
