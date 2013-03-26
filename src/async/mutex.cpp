#include "async/mutex.hpp"
#include "async/impl/mutex.hpp"

#include <cassert>

namespace async
{
    Mutex::Mutex(bool recursive)
        : _implMutex(new impl::Mutex(recursive))
    {
    }

    Mutex::Mutex(const Mutex &other)
        : _implMutex(other._implMutex)
    {
    }

    Mutex::Mutex(Mutex &&other)
        : _implMutex(std::move(other._implMutex))
    {
    }

    Mutex::~Mutex()
    {
    }

    Mutex &Mutex::operator=(const Mutex &other)
    {
        _implMutex = other._implMutex;
        return *this;
    }

    Mutex &Mutex::operator=(Mutex &&other)
    {
        Mutex(std::move(other)).swap(*this);
        return *this;
    }

    void Mutex::swap(Mutex &other)
    {
        _implMutex.swap(other._implMutex);
    }

    bool Mutex::tryLock()
    {
        return _implMutex->tryLock();
    }

    bool Mutex::isLocked()
    {
        return _implMutex->isLocked();
    }

    void Mutex::unlock()
    {
        return _implMutex->unlock();
    }
}
