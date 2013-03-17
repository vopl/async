#include "async/stable.hpp"
#include "async/codeManager.hpp"
#include "async/impl/scheduler.hpp"
#include "async/scheduler.hpp"

#include <cassert>

namespace async
{
    CodeManager::CodeManager()
        : _implScheduler(new impl::Scheduler)
    {
    }

    CodeManager::CodeManager(const impl::SchedulerPtr &implScheduler)
        : _implScheduler(implScheduler)
    {
    }

    CodeManager::operator Scheduler()
    {
        return Scheduler(_implScheduler);
    }

    void CodeManager::spawn(const std::function<void()> &code)
    {
        return _implScheduler->spawn(code);
    }
}
