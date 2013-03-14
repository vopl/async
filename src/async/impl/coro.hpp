#ifndef _ASYNC_IMPL_CORO_HPP_
#define _ASYNC_IMPL_CORO_HPP_

#include <memory>
#include "async/impl/contextEngine.hpp"

namespace async { namespace impl
{
    class Scheduler;

    class Coro
        : public std::enable_shared_from_this<Coro>
    {

    public:
        Coro(Scheduler *scheduler, size_t stackSize=1024*32);
        ~Coro();

        bool hasCode();
        void setCode(const std::function<void()> &code);

        void activate();

    private:
        Scheduler *_scheduler;
        std::function<void()> _code;

    private:
        friend class ContextEngine;
        ContextEngine::Context _context;

        void contextProc();
    };

    typedef std::shared_ptr<Coro> CoroPtr;
}}

#endif
