#ifndef _ASYNC_IMPL_CONTEXTENGINE_HPP_
#define _ASYNC_IMPL_CONTEXTENGINE_HPP_

#include "async/config.h"

#if ASYNCSCHEDULER_CONTEXTENGINE_UCONTEXT
#   include <ucontext.h>
#else
#   error not have ucontext.h
#endif

#include <mutex>
#include <atomic>

#if defined(HAVE_VALGRIND)
#   define USE_VALGRIND
#endif

namespace async { namespace impl
{
    class Coro;
    class Thread;

    class ContextEngine
    {

    public:
        ContextEngine();
        ~ContextEngine();

    public://for thread
        bool te_init(Thread *thread);
        void te_deinit();

    public://for task
        struct Context
            : ucontext_t
        {
#if defined(USE_VALGRIND)
            int _valgrindStackId;
#endif
        };

    public://for task
        static void contextCreate(Coro *coro, size_t stackSize);
        static void contextActivate(Coro *coro);
        static void contextDeactivate(Coro *coro, std::atomic<uint32_t> *atomicForSetAfter, uint32_t valueForSetAfter);
        static void contextDestroy(Coro *coro);

    private:
        static __thread Context _rootContext;
        static __thread Context *_currentContext;
        static __thread std::atomic<uint32_t> *_atomicForSetAfter;
        static __thread uint32_t _valueForSetAfter;

    private:
#if PVOID_SIZE == INT_SIZE
        static void s_contextProc(int param);
#elif PVOID_SIZE == INT_SIZE*2
        static void s_contextProc(int param1, int param2);
#else
#   error PVOID_SIZE not equal INT_SIZE or INT_SIZE*2
#endif

    };
}}

#endif
