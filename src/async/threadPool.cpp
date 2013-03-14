#include "async/stable.hpp"
#include "async/threadPool.hpp"
#include <cassert>

namespace async
{
    ThreadPool::ThreadPool(const async::ThreadUtilizer &tu, size_t amount)
        : _tu(tu)
    {
        _threads.resize(amount);
        for(std::thread &thread: _threads)
        {
            thread = std::thread([this]{_tu.te_utilize();});
        }
        //TODO ждать пока все воркеры окончат регистрацию в шедулере
    }

    ThreadPool::~ThreadPool()
    {
        for(std::thread &thread: _threads)
        {
            assert(thread.get_id() != std::this_thread::get_id());
            EThreadReleaseResult etrr = _tu.release(thread.native_handle());
            assert(etrr_ok == etrr);
        }
        for(std::thread &thread: _threads)
        {
            thread.join();
        }
        _threads.clear();
    }
}
