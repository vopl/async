#include "async/stable.hpp"
#include "async/impl/threadContainer.hpp"

#include <cassert>

namespace async { namespace impl
{
    ThreadContainer::ThreadContainer()
    {
    }

    ThreadContainer::~ThreadContainer()
    {
        assert(_threads.empty());
    }

    bool ThreadContainer::te_init(ThreadController *controller)
    {
        std::unique_lock<std::mutex> l(_mtx);

        std::pair<TMThreads::iterator, bool> insertRes =
                _threads.insert(std::make_pair(std::this_thread::get_id(), controller));

        return insertRes.second;
    }

    extern std::atomic<size_t> g_counter;

    ContextPtr ThreadContainer::te_emitWorkPiece()
    {
        ContextPtr ctx(new Context);
        ctx->setCode([]{
            volatile int c(0);
            for(int k(0); k<10; k++)
            {
                c+=k;
            }
            async::impl::g_counter++;
        });
        return ctx;
    }

    void ThreadContainer::te_deinit()
    {
        std::unique_lock<std::mutex> l(_mtx);

        TMThreads::iterator iter = _threads.find(std::this_thread::get_id());

        if(_threads.end() == iter)
        {
            return;
        }

        _threads.erase(iter);
    }

    EThreadReleaseResult ThreadContainer::release(const std::thread::id &id)
    {
        std::unique_lock<std::mutex> l(_mtx);
        TMThreads::iterator iter = _threads.find(id);

        if(_threads.end() == iter)
        {
            return etrr_notInWork;
        }

        iter->second->pushReleaseRequest();
        return etrr_ok;
    }

}}
