#include "async/impl/synchronizer.hpp"
#include "async/impl/coro.hpp"
#include "async/impl/scheduler.hpp"

#include <cassert>

namespace async { namespace impl
{
    Synchronizer::Synchronizer()
    {

    }

    Synchronizer::~Synchronizer()
    {
        assert(_holds.empty());
        for(const CoroPtr &coro : _holds)
        {
            coro->scheduler()->coroReadyIfHolded(coro.get());
        }
    }

    size_t Synchronizer::holdsAmount()
    {
        assert(!_mtx.try_lock() && "must be already locked");
        return _holds.size();
    }

    void Synchronizer::activateOthers(size_t holdsAmount)
    {
        assert(!_mtx.try_lock() && "must be already locked");

        if(holdsAmount >= _holds.size())
        {
            for(CoroPtr &coro : _holds)
            {
                coro->scheduler()->coroReadyIfHolded(coro.get());
                coro.reset();
            }
            _holds.clear();
        }
        else if(holdsAmount)
        {
            for(size_t idx(0); idx<holdsAmount; idx++)
            {
                CoroPtr &coro = _holds[idx];
                coro->scheduler()->coroReadyIfHolded(coro.get());
                coro.reset();
            }
            _holds.erase(_holds.begin(), _holds.begin()+holdsAmount);
        }
    }

    void Synchronizer::holdSelf()
    {
        assert(!_mtx.try_lock() && "must be already locked");

        Coro *coro = Coro::current();
        assert(coro);
        if(!coro)
        {
            throw !"must be called in async context";
        }

        Scheduler *scheduler = coro->scheduler();

        assert(holdsAmount() < 200);
        _holds.push_back(coro->shared_from_this());

        scheduler->contextDeactivate(coro, &_mtx);
    }


}}
