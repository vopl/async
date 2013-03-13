#include "async/impl/context.hpp"

namespace async { namespace impl
{

    Context::Context(size_t stackSize)
    {

    }

    Context::~Context()
    {

    }

    bool Context::hasCode()
    {
        return _code.operator bool();
    }

    bool Context::setCode(const std::function<void()> &code)
    {
        if(!_code)
        {
            _code = code;
            return true;
        }

        return false;
    }

    Context::EActivationResult Context::activate()
    {
        if(!_code)
        {
            return ear_failNoCode;
        }
        _code();
        (std::function<void()>()).swap(_code);
        return ear_okComplete;
    }
}}
