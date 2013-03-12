#include "async/stable.hpp"
#include "async/impl/threadControllerSet.hpp"

#include <cassert>

namespace async { namespace impl
{
    ThreadControllerSet::ThreadControllerSet()
    {
    }

    ThreadControllerSet::~ThreadControllerSet()
    {
        assert(_threads.empty());
    }

    bool ThreadControllerSet::te_init(ThreadController *controller)
    {
        std::unique_lock<std::mutex> l(_mtx);

        std::pair<TMThreads::iterator, bool> insertRes =
                _threads.insert(std::make_pair(std::this_thread::get_id(), controller));

        return insertRes.second;
    }

    void *ThreadControllerSet::te_emitWorkPiece()
    {
        //assert(0);
        return this;
    }

    void ThreadControllerSet::te_deinit()
    {
        std::unique_lock<std::mutex> l(_mtx);

        TMThreads::iterator iter = _threads.find(std::this_thread::get_id());

        if(_threads.end() == iter)
        {
            return;
        }

        _threads.erase(iter);
    }

    EThreadReleaseResult ThreadControllerSet::threadRelease(const std::thread::id &id)
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
