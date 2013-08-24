#include "async/mutex.hpp"
#include "async/impl/mutex.hpp"

#include <cassert>

namespace async
{
    Mutex::Mutex(bool recursive)
        : Base(recursive)
    {
    }

    Mutex::~Mutex()
    {
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
