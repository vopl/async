#include "async/mutex.hpp"
#include "async/impl/mutex.hpp"

#include <cassert>

namespace async
{
    Mutex::Mutex()
        : Base()
    {
    }

    Mutex::~Mutex()
    {
    }

    void Mutex::lock()
    {
        return Base::impl().lock();
    }

    bool Mutex::tryLock()
    {
        return Base::impl().tryLock();
    }

    bool Mutex::isLocked()
    {
        return Base::impl().isLocked();
    }

    void Mutex::unlock()
    {
        return Base::impl().unlock();
    }
}
