#ifndef _ASYNC_IMPL_TASKCONTAINER_HPP_
#define _ASYNC_IMPL_TASKCONTAINER_HPP_

#include "async/impl/task.hpp"

#include <mutex>
#include <queue>
#include <vector>
#include <set>

namespace async { namespace impl
{
    class TaskContainer
    {

    public:
        TaskContainer();
        ~TaskContainer();

    public://for thread
        Task* te_emitWorkPiece();

    public://for busines
        void pushCodeToRun(const std::function<void()> &code);

    public://for context
        void markTaskAsExec(Task *ctx);
        void markTaskAsHold(Task *ctx);
        void markTaskAsEmpty(Task *ctx);

    private:

        std::mutex _mtx;
        std::queue<TaskPtr>     _ready;//fifo
        std::vector<TaskPtr>	_empty;//push popAny
        std::set<TaskPtr>       _hold;//insert search-pop
        std::set<TaskPtr>       _exec;//insert search-pop
        std::set<TaskPtr>       _emitted;//insert search-pop
    };
}}

#endif
