#ifndef _ASYNC_IMPL_THREAD_HPP_
#define _ASYNC_IMPL_THREAD_HPP_

#include "async/threadUtilizer.hpp"
#include "async/impl/task.hpp"

#include <mutex>
#include <condition_variable>
#include <cassert>


namespace async { namespace impl
{
    class Scheduler;

    class Thread
    {
    public:
        Thread(Scheduler *scheduler);
        ~Thread();

        template <class LimitCounter>
        EThreadUtilizationResult utilize(LimitCounter &limitCounter);

    public:
        bool pushWorkPiece(Task *workPiece);
        void pushReleaseRequest();

    private:
        void requestWorkPiece();

    private:
        Scheduler *_scheduler;

        std::mutex _mtx;
        std::condition_variable _cv;

        Task* _workPiece;
        bool _releaseRequest;
    };

    template <class LimitCounter>
    EThreadUtilizationResult Thread::utilize(LimitCounter &limitCounter)
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
                Task* workPiece(NULL);
                std::swap(workPiece, _workPiece);
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
