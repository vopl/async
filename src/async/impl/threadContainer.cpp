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

    bool ThreadContainer::te_init(Thread *thread)
    {
        std::unique_lock<std::mutex> l(_mtx);

        std::pair<TMThreads::iterator, bool> insertRes =
                _threads.insert(std::make_pair(std::this_thread::get_id(), thread));

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

        _threads.erase(iter);
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

    bool ThreadContainer::pushWorkPiece(Task *workPiece)
    {
		//cycled for each threads, if push then return true

    	assert(0);
    	return false;
    }

}}
