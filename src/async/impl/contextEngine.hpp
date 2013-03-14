#ifndef _ASYNC_IMPL_CONTEXTENGINE_HPP_
#define _ASYNC_IMPL_CONTEXTENGINE_HPP_

#include "config.h"

#if defined(HAVE_UCONTEXT_H)
#   include <ucontext.h>
#else
#   error not have ucontext.h
#endif


#if defined(HAVE_VALGRIND)
#   define USE_VALGRIND
#endif

namespace async { namespace impl
{
    class Context;
    class Thread;

    class ContextEngine
    {

    public:
        ContextEngine();
        ~ContextEngine();

    public://for thread
        bool te_init(Thread *thread);
        void te_deinit();

    public://for context
        struct ContextState
        {
            ucontext_t _uctx;
#if defined(USE_VALGRIND)
            int _valgrindStackId;
#endif
        };

    public://for context
        void contextCreate(ContextState *ctx, size_t stackSize);
        void contextActivate(ContextState *ctx);
        void contextDestroy(ContextState *ctx);
    };
}}

#endif
