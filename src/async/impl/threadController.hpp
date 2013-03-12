#ifndef _ASYNC_IMPL_THREADCONTROLLER_HPP_
#define _ASYNC_IMPL_THREADCONTROLLER_HPP_

#include "async/threadUtilizer.hpp"
#include <mutex>
#include <condition_variable>

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
        bool pushWorkPiece(void *workPiece);
        void pushReleaseRequest();

    private:
        void requestWorkPiece();

    private:
        Scheduler *_scheduler;

        std::mutex _mtx;
        std::condition_variable _cv;

        void *_workPiece;
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
            if(_workPiece)
            {
                _workPiece = NULL;
                lock.unlock();
                //doWork;
                lock.lock();
                limitCounter.incrementWorkAmount();
            }

            if(_releaseRequest)
            {
                return etur_releaseRequest;
            }

            if(limitCounter.completed())
            {
                return etur_limitExhausted;
            }

            requestWorkPiece();

            if(!_workPiece)
            {
                limitCounter.wait(_cv, lock);
            }
        }
    }

}}

#endif
