#ifndef _ASYNC_IMPL_TASK_HPP_
#define _ASYNC_IMPL_TASK_HPP_

#include <memory>
#include "async/impl/contextEngine.hpp"

namespace async { namespace impl
{
    class Scheduler;

    class Task
        : public std::enable_shared_from_this<Task>
    {

    public:
        Task(Scheduler *scheduler, size_t stackSize=1024*32);
        ~Task();

        bool hasCode();
        bool setCode(const std::function<void()> &code);

        void activate();

    private:
        Scheduler *_scheduler;
        std::function<void()> _code;

    private:
        friend class ContextEngine;
        ContextEngine::Context _context;

        void contextProc();
    };

    typedef std::shared_ptr<Task> TaskPtr;
}}

#endif
