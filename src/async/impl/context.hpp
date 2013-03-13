#ifndef _ASYNC_IMPL_CONTEXT_HPP_
#define _ASYNC_IMPL_CONTEXT_HPP_

#include <memory>

namespace async { namespace impl
{
    class Scheduler;

    class Context
    	: public std::enable_shared_from_this<Context>
    {

    public:
        Context(Scheduler *scheduler, size_t stackSize=1024*32);
        ~Context();

        bool hasCode();
        bool setCode(const std::function<void()> &code);

        void activate();

    private:
        Scheduler *_scheduler;
        std::function<void()> _code;
    };

    typedef std::shared_ptr<Context> ContextPtr;
}}

#endif
