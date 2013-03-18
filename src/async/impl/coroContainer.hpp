#ifndef _ASYNC_IMPL_COROCONTAINER_HPP_
#define _ASYNC_IMPL_COROCONTAINER_HPP_

#include "async/impl/coro.hpp"

#include <mutex>
#include <queue>
#include <vector>
#include <set>

namespace async { namespace impl
{
    class CoroContainer
    {

    public:
        CoroContainer();
        ~CoroContainer();

    public://for thread
        Coro* te_emitWorkPiece();

    public://for busines
        void spawn(const std::function<void()> &code);

    public://for context
        void coroActivate(Coro *coro);
        void coroHold(Coro *coro);
        void coroReadyIfHolded(Coro *coro);

        static Coro *coroCurrent();


    private:

        static __thread Coro *_current;


        std::mutex _mtx;

        //boost::multi_index {kind, ptr}
        //fifo for ready
        std::queue<CoroPtr>     _ready;//fifo
        std::vector<CoroPtr>	_empty;//push popAny
        std::set<CoroPtr>       _hold;//insert search-pop
        std::set<CoroPtr>       _exec;//insert search-pop
        std::set<CoroPtr>       _emitted;//insert search-pop
    };
}}

#endif
