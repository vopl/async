#include "async/mutex.hpp"
#include "async/impl/mutex.hpp"

#include <cassert>

namespace async
{
    Mutex::Mutex(bool recursive)
    {
        Base::ctor(recursive);
    }

    Mutex::~Mutex()
    {
        Base::dtor();
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
