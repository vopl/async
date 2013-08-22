#ifndef _ASYNC_HIDDENIMPL_HPP_
#define _ASYNC_HIDDENIMPL_HPP_

#include <memory>
#include "async/sizeofImpl.hpp"

namespace async
{
    template <class T>
    class HiddenImpl
    {
    protected:
        template <typename... Arg>
        HiddenImpl(const Arg &... args)
        {
            new (&_data) T(args...);
        }

        ~HiddenImpl()
        {
            (&impl())->~T();
        }

        T &impl()
        {
            return *static_cast<T*>(static_cast<void*>(&_data));
        }

    private:
        std::aligned_storage<sizeofImpl<T>::_value> _data;
    };
}

#endif
