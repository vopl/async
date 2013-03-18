#include "async/stable.hpp"
#include "async/impl/threadContainer.hpp"
#include "async/impl/thread.hpp"

#include <cassert>

namespace async { namespace impl
{
    ThreadContainer::ThreadContainer()
        : _nextThreadForPushWork(_threads.end())
    {
    }

    ThreadContainer::~ThreadContainer()
    {
        assert(_threads.empty());
    }

    bool ThreadContainer::te_init(Thread *thread)
    {
        std::unique_lock<std::mutex> l(_mtx);

        std::pair<TMThreads::iterator, bool> insertRes =
                _threads.insert(std::make_pair(std::this_thread::get_id(), thread));

        _nextThreadForPushWork = insertRes.first;
        return insertRes.second;
    }

    void ThreadContainer::te_deinit()
    {
        std::unique_lock<std::mutex> l(_mtx);

        TMThreads::iterator iter = _threads.find(std::this_thread::get_id());

        if(_threads.end() == iter)
        {
            return;
        }

        _nextThreadForPushWork = _threads.erase(iter);
    }

    EThreadReleaseResult ThreadContainer::releaseThread(const std::thread::id &id)
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

    bool ThreadContainer::pushWorkPiece(Coro *workPiece)
    {
        std::unique_lock<std::mutex> l(_mtx);

        size_t threadsAmount = _threads.size();
        for(size_t i(0); i<threadsAmount; i++)
        {
            if(_threads.end() == _nextThreadForPushWork)
            {
                _nextThreadForPushWork = _threads.begin();
            }

            Thread *thread = _nextThreadForPushWork->second;

            ++_nextThreadForPushWork;

            if(thread->pushWorkPiece(workPiece))
            {
                return true;
            }
        }

        return false;
    }

}}
