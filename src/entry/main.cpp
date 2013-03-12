#include "async/threadUtilizer.hpp"
#include "async/threadPool.hpp"
#include "async/scheduler.hpp"
#include <iostream>
#include <thread>
#include <cassert>
#include <vector>
#include <atomic>

namespace async { namespace impl
{
    std::atomic<size_t> g_counter(0);
}}


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

    for(size_t k(0); k<40; k++)
    {
        async::impl::g_counter = 0;

        if(!(k%10))
        {
            std::atomic<bool> exit(false);

            std::thread t([&exit]
                {
                    while(!exit)
                    {
                        volatile size_t c(0);
                        for(size_t i(0); i<10000; i++)
                        {
                            c+=i;
                        }
                        async::impl::g_counter++;
                    }
                });

            std::this_thread::sleep_for(std::chrono::seconds(1));
            exit = true;
            t.join();
        }
        else
        {
            async::ThreadPool tp(tu, (k%10));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout<<((k%10))<<" "<<async::impl::g_counter<<"\n"; std::cout.flush();
    }

    return 0;
}
