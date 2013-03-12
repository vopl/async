#ifndef _ASYNC_THREADUTILIZER_HPP_
#define _ASYNC_THREADUTILIZER_HPP_

#include <chrono>
#include <thread>

namespace async
{

    enum EThreadUtilizationResult
    {
        etur_kernelStop,
        etur_limitExhausted,
        etur_releaseRequest,
        etur_alreadyInWork  //was not be utilized
    };

    enum EThreadReleaseResult
    {
        etrr_ok,
        etrr_notInWork
    };

    class Scheduler;

    namespace impl
    {
        class Scheduler;
        typedef std::shared_ptr<Scheduler> SchedulerPtr;
    }

    class ThreadUtilizer
    {
    public:
        typedef std::chrono::high_resolution_clock _clock;

    public:
        ThreadUtilizer();
        ThreadUtilizer(const impl::SchedulerPtr &implScheduler);

        operator Scheduler();

    public:
        EThreadUtilizationResult te_utilize();
        EThreadUtilizationResult te_utilize(const _clock::duration &duration);
        EThreadUtilizationResult te_utilize(const _clock::time_point &atime);
        EThreadUtilizationResult te_utilize(const size_t &workPiecesAmount);

        EThreadReleaseResult te_release();
        EThreadReleaseResult release(std::thread::native_handle_type id);

    public:
        template<typename rep, typename period>
        EThreadUtilizationResult te_utilize(const std::chrono::duration<rep, period> &duration);

        template<typename clock, typename duration>
        EThreadUtilizationResult te_utilize(const std::chrono::time_point<clock, duration> &atime);

    private:
        impl::SchedulerPtr _implScheduler;
    };


    ////////////////////////////////////////////////////////////////////////////////
    template<typename rep, typename period>
    EThreadUtilizationResult ThreadUtilizer::te_utilize(const std::chrono::duration<rep, period> &duration)
    {
        return te_utilize(std::chrono::duration_cast<_clock::duration>(duration));
    }

    template<typename clock, typename duration>
    EThreadUtilizationResult ThreadUtilizer::te_utilize(const std::chrono::time_point<clock, duration> &atime)
    {
        return te_utilize(atime - clock::now());
    }

}

#endif
