#ifndef _ASYNC_IMPL_COROCONTAINER_HPP_
#define _ASYNC_IMPL_COROCONTAINER_HPP_

#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <set>

namespace async { namespace impl
{
    class Coro;

    class CoroContainer
    {

    public:
        CoroContainer();
        ~CoroContainer();

    public://for thread
        Coro* te_emitWorkPiece();

    public://for busines
        void spawn(const std::function<void()> &code);
        void spawn(std::function<void()> &&code);

    public://for context
        void markCoroExec(Coro *coro);
        void markCoroHold(Coro *coro);
        void markCoroComplete(Coro *coro);

        void coroReady(Coro *coro);

        static Coro *coroCurrent();

    private:

        static __thread Coro *_current;


        std::mutex _mtx;

        //boost::multi_index {kind, ptr}
        //fifo for ready
        std::queue<Coro*>     _ready;//fifo
        std::vector<Coro*>	_empty;//push popAny
    };
}}

#endif
