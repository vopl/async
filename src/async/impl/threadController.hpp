#ifndef _ASYNC_IMPL_THREADCONTROLLER_HPP_
#define _ASYNC_IMPL_THREADCONTROLLER_HPP_

#include "async/threadUtilizer.hpp"

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

    extern std::atomic<size_t> g_counter;

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
                _workPiece = NULL;
                lock.unlock();

                //do work
                {
                    volatile size_t c(0);
                    for(size_t i(0); i<100; i++)
                    {
                        c+=i;
                    }
                    g_counter++;
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
