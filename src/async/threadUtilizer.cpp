#include "async/threadUtilizer.hpp"
#include "async/impl/scheduler.hpp"
#include "async/impl/threadController.hpp"
#include <cassert>

#include <mutex>

namespace async
{
    ThreadUtilizer::ThreadUtilizer()
        : _implScheduler(new impl::Scheduler)
    {
    }

    ThreadUtilizer::ThreadUtilizer(const impl::SchedulerPtr &implScheduler)
        : _implScheduler(implScheduler)
    {
    }

    EThreadUtilizationResult ThreadUtilizer::te_utilize()
    {
        struct LimitCounter
        {
            void incrementWorkAmount()
            {
            }

            bool completed()
            {
                return false;
            }

            void wait(std::condition_variable &cv, std::unique_lock<std::mutex> &lock)
            {
                cv.wait(lock);
            }
        } limitCounter;

        impl::ThreadController tc(_implScheduler.get());
        return tc.utilize(limitCounter);
    }

    EThreadUtilizationResult ThreadUtilizer::te_utilize(const _clock::duration &duration)
    {
        return te_utilize(_clock::now() + duration);
    }

    EThreadUtilizationResult ThreadUtilizer::te_utilize(const _clock::time_point &time)
    {
        struct LimitCounter
        {
            const _clock::time_point &_time;
            std::cv_status _lastWaitStatus = std::cv_status::no_timeout;

            LimitCounter(const _clock::time_point &time)
                : _time(time)
            {
            }

            void incrementWorkAmount()
            {
            }

            bool completed()
            {
                return std::cv_status::timeout == _lastWaitStatus;
            }

            void wait(std::condition_variable &cv, std::unique_lock<std::mutex> &lock)
            {
                _lastWaitStatus = cv.wait_until(lock, _time);
            }
        } limitCounter(time);

        impl::ThreadController tc(_implScheduler.get());
        return tc.utilize(limitCounter);
    }

    EThreadUtilizationResult ThreadUtilizer::te_utilize(const size_t &workPiecesAmount)
    {
        struct LimitCounter
        {
            const size_t &_limit;
            size_t _counter=0;

            LimitCounter(const size_t &limit)
                : _limit(limit)
            {
            }

            void incrementWorkAmount()
            {
                _counter++;
            }

            bool completed()
            {
                return _counter >= _limit;
            }

            void wait(std::condition_variable &cv, std::unique_lock<std::mutex> &lock)
            {
                cv.wait(lock);
            }
        } limitCounter(workPiecesAmount);

        impl::ThreadController tc(_implScheduler.get());
        return tc.utilize(limitCounter);
    }

    EThreadReleaseResult ThreadUtilizer::te_release()
    {
        return _implScheduler->threadRelease(std::this_thread::get_id());
    }

    EThreadReleaseResult ThreadUtilizer::release(std::thread::native_handle_type id)
    {
        return _implScheduler->threadRelease(std::thread::id(id));
    }

}


