#include "async/scheduler.hpp"
#include "async/threadUtilizer.hpp"
#include "async/codeManager.hpp"
#include "async/threadPool.hpp"
#include "async/event.hpp"
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
        async::ThreadPool tp(tu, 4);

        async::Event event;

        for(size_t k(0); k<30000; k++)
        {
            cm.spawn([k, &event]{
                //std::cout<<"start test "<<k<<std::endl;
                //std::this_thread::sleep_for(std::chrono::milliseconds(1));

                char tmp[32];
//                if(!(k&1))
                if((k&1))
                {
//                    sprintf(tmp, "set        %p\n", async::impl::Coro::current());
//                    std::cout<<tmp; std::cout.flush();
                    event.set(async::Event::erm_afterNotifyAll);
//                    sprintf(tmp, "after set  %p\n", async::impl::Coro::current());
//                    std::cout<<tmp; std::cout.flush();
                }
                else
                {
//                    sprintf(tmp, "wait       %p\n", async::impl::Coro::current());
//                    std::cout<<tmp; std::cout.flush();
                    event.wait();
//                    sprintf(tmp, "after wait %p\n", async::impl::Coro::current());
//                    std::cout<<tmp; std::cout.flush();
                }

            });
        }

        for(size_t k(0); k<10; k++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            event.set(async::Event::erm_afterNotifyAll);
        }

//        for(size_t k(0); k<1000; k++)
//        {
//            tu.te_utilize(std::chrono::nanoseconds(0));
//        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
