#include "async/threadUtilizer.hpp"
#include "async/threadPool.hpp"
#include "async/scheduler.hpp"
#include <iostream>
#include <thread>
#include <cassert>
#include <vector>


int main()
{
    async::Scheduler sched;
    async::ThreadUtilizer tu(sched);

    if(0)
    {
        tu.te_utilize(std::chrono::nanoseconds(200));
        tu.te_utilize(std::chrono::minutes(200));
        tu.te_utilize(std::chrono::system_clock::now()+std::chrono::minutes(200));

        std::thread t(
                    [&t,&tu] ()
                    {
                        async::EThreadUtilizationResult etur = tu.te_utilize(std::chrono::nanoseconds(200));
                        assert(async::etur_limitExhausted == etur);

                        assert(async::etrr_notInWork == tu.te_release());
                        assert(async::etrr_notInWork == tu.release(t.native_handle()));
                    });

        t.join();
        assert(async::etrr_notInWork == tu.release(t.native_handle()));
    }

    {
        async::ThreadPool tp(tu);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}
