#include <iostream>

#include "async/scheduler.hpp"
#include "async/threadUtilizer.hpp"
#include "async/codeManager.hpp"
#include "async/threadPool.hpp"
#include "async/event.hpp"
#include "async/mutex.hpp"
#include "async/wait.hpp"
#include "async/impl/coro.hpp"

#include <iostream>
#include <thread>
#include <cassert>
#include <vector>
#include <atomic>

int main()
{
    async::Scheduler sched;
    async::ThreadUtilizer tu(sched);
    async::CodeManager cm(sched);

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
        async::ThreadPool tp(tu, 1);

        std::atomic<size_t> cnt(0);
        size_t amount = 3000000;
        async::Event event(true);
        async::Event event2(true);
        async::Mutex mutex;
        async::Mutex mutex2;

        for(size_t k(0); k<amount; k++)
        {
            //if(! (k%2))
            {
                char tmp[32];
                sprintf(tmp, "iter (%d)\n", (int)k);
                std::cout<<tmp; std::cout.flush();
            }

            cm.spawn([k, &event, &event2, &cnt, &mutex, &mutex2]{
                char tmp[32];
                (void)tmp;
//                if(!(k&1))
                if((k&1))
                {
//                    sprintf(tmp, "pre set        %p\n", async::impl::Coro::current());
//                    std::cout<<tmp; std::cout.flush();

                    async::waitAny(mutex);

//                    sprintf(tmp, "set        %p\n", async::impl::Coro::current());
//                    std::cout<<tmp; std::cout.flush();

                    size_t i=rand() > RAND_MAX/2;
                    if(!i)
                    {
                        event.set();
                    }
                    else
                    {
                        event2.set();
                    }

//                    sprintf(tmp, "after set  %p(%d)\n", async::impl::Coro::current(), (int)i);
//                    std::cout<<tmp; std::cout.flush();

                    mutex.unlock();
                }
                else
                {
//                    sprintf(tmp, "pre wait        %p\n", async::impl::Coro::current());
//                    std::cout<<tmp; std::cout.flush();

                    //async::waitAny(mutex2);
                    mutex2.lock();

//                    sprintf(tmp, "wait       %p\n", async::impl::Coro::current());
//                    std::cout<<tmp; std::cout.flush();

                    uint32_t i = async::waitAny(event, event2);

//                    sprintf(tmp, "after wait %p (%d)\n", async::impl::Coro::current(), (int)i);
//                    std::cout<<tmp; std::cout.flush();

                    mutex2.unlock();
                }

                cnt++;
            });
        }

        while(cnt < amount)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            size_t i=rand() > RAND_MAX/2;
            if(!i)
            {
                event.set();
            }
            else
            {
                event2.set();
            }
        }

        std::cout<<"done"<<std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
