#ifndef _ASYNC_IMPL_CONTEXT_HPP_
#define _ASYNC_IMPL_CONTEXT_HPP_

#include <memory>

namespace async { namespace impl
{
    class Context
    {

    public:
        Context(size_t stackSize=1024*32);
        ~Context();

        bool hasCode();
        bool setCode(const std::function<void()> &code);

        enum EActivationResult
        {
            ear_okIncomplete,
            ear_okComplete,
            ear_failNoCode
        };

        EActivationResult activate();

    private:
        std::function<void()> _code;
    };

    typedef std::shared_ptr<Context> ContextPtr;
}}

#endif
