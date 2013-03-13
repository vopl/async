#include "async/stable.hpp"
#include "async/threadUtilizer.hpp"
#include "async/impl/scheduler.hpp"
#include "async/impl/thread.hpp"
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

        impl::Thread tc(_implScheduler.get());
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
            std::cv_status _lastWaitStatus;

            LimitCounter(const _clock::time_point &time)
                : _time(time)
            	, _lastWaitStatus(std::cv_status::no_timeout)
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

        impl::Thread tc(_implScheduler.get());
        return tc.utilize(limitCounter);
    }

    EThreadUtilizationResult ThreadUtilizer::te_utilize(const size_t &workPiecesAmount)
    {
        struct LimitCounter
        {
            const size_t &_limit;
            size_t _counter;

            LimitCounter(const size_t &limit)
                : _limit(limit)
            	, _counter(0)
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

        impl::Thread tc(_implScheduler.get());
        return tc.utilize(limitCounter);
    }

    EThreadReleaseResult ThreadUtilizer::te_release()
    {
        return _implScheduler->releaseThread(std::this_thread::get_id());
    }

    EThreadReleaseResult ThreadUtilizer::release(std::thread::native_handle_type id)
    {
        return _implScheduler->releaseThread(std::thread::id(id));
    }

}


