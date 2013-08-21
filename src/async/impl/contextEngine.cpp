#include "stable.hpp"
#include "async/impl/contextEngine.hpp"
#include "async/impl/thread.hpp"
#include "async/impl/coro.hpp"

#include <cassert>
#if defined(USE_VALGRIND)
#   include <valgrind.h>
#endif

namespace async { namespace impl
{
    __thread ContextEngine::Context ContextEngine::_rootContext = {};
    __thread ContextEngine::Context *ContextEngine::_currentContext = nullptr;
    __thread std::atomic<size_t> *ContextEngine::_atomicForSetAfter = nullptr;
    __thread size_t ContextEngine::_valueForSetAfter = 0;

    ContextEngine::ContextEngine()
	{
    }

	ContextEngine::~ContextEngine()
	{
	}

	bool ContextEngine::te_init(Thread *thread)
	{
        (void)thread;

        //create root context
        Context *ctx = &_rootContext;
        assert(!ctx->uc_stack.ss_sp);

        if(getcontext(ctx))
        {
            return false;
        }
        _currentContext = ctx;

        return true;
	}

	void ContextEngine::te_deinit()
	{
		//destroy root context

        //nothing
        assert(_currentContext == &_rootContext);
        _currentContext = nullptr;
    }

    void ContextEngine::contextCreate(Coro *coro, size_t stackSize)
    {
        Context *ctx = &coro->_context;

        if(getcontext(ctx))
        {
            std::cerr<<__FUNCTION__<<", getcontext failed"<<std::endl;
            std::terminate();
            return;
        }

        ctx->uc_link = NULL;
        ctx->uc_stack.ss_sp = (char *)malloc(stackSize);
        ctx->uc_stack.ss_size = stackSize;

#if defined(USE_VALGRIND)
        ctx->_valgrindStackId = VALGRIND_STACK_REGISTER(ctx->uc_stack.ss_sp, (char *)ctx->uc_stack.ss_sp + ctx->uc_stack.ss_size);
#endif

#if PVOID_SIZE == INT_SIZE
        static_assert(sizeof(int) == sizeof(task), "sizeof(int) == sizeof(task)");
        int iarg = static_cast<int>(task);
        makecontext(ctx, (void (*)(void))&ContextEngine::s_contextProc, 1, iarg);
#elif PVOID_SIZE == INT_SIZE*2
        static_assert(sizeof(uint64_t) == sizeof(coro), "sizeof(uint64_t) == sizeof(task)");
        uint64_t iarg = reinterpret_cast<uint64_t>(coro);
        int iarg1 = (unsigned int)(iarg&0xffffffff);
        int iarg2 = (unsigned int)((iarg>>32)&0xffffffff);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
        makecontext(ctx, (void (*)(void))&ContextEngine::s_contextProc, 2, iarg1, iarg2);
#pragma GCC diagnostic pop

#else
        error PVOID_SIZE not equal INT_SIZE or INT_SIZE*2
#endif

    }

    void ContextEngine::contextActivate(Coro *coro)
	{
        assert(_currentContext);
        assert(_currentContext == &_rootContext);
        assert(_currentContext != &coro->_context);

        coro->scheduler()->markCoroExec(coro);

        _currentContext = &coro->_context;
        swapcontext(&_rootContext, &coro->_context);

        if(coro->hasCode())
        {
        	coro->scheduler()->markCoroHold(coro);
        }
        else
        {
        	coro->scheduler()->markCoroComplete(coro);
        }

        if(_atomicForSetAfter)
        {
            _atomicForSetAfter->store(_valueForSetAfter);
            _atomicForSetAfter = nullptr;
            _valueForSetAfter = 0;
        }
	}

    void ContextEngine::contextDeactivate(Coro *coro, std::atomic<size_t> *atomicForSetAfter, size_t valueForSetAfter)
    {
        assert(_currentContext);
        assert(_currentContext != &_rootContext);
        assert(_currentContext == &coro->_context);

        assert(!_atomicForSetAfter);
        assert(!_valueForSetAfter);

        _atomicForSetAfter = atomicForSetAfter;
        _valueForSetAfter = valueForSetAfter;

        _currentContext = &_rootContext;
        swapcontext(&coro->_context, &_rootContext);

        assert(!_atomicForSetAfter);
        assert(!_valueForSetAfter);
    }

    void ContextEngine::contextDestroy(Coro *coro)
    {
        Context *ctx = &coro->_context;
        assert(_currentContext != ctx);

        if(ctx->uc_stack.ss_sp)
        {
#if defined(USE_VALGRIND)
            VALGRIND_STACK_DEREGISTER(ctx->_valgrindStackId);
#endif
            free(ctx->uc_stack.ss_sp);
        }
    }

#if PVOID_SIZE == INT_SIZE
    void ContextEngine::s_contextProc(int param)
    {
        reinterpret_cast<Task*>(param)->contextProc();
    }
#elif PVOID_SIZE == INT_SIZE*2
    void ContextEngine::s_contextProc(int param1, int param2)
    {
        uint64_t icoro = ((unsigned int)param1) | (((uint64_t)param2)<<32);

        reinterpret_cast<Coro*>(icoro)->contextProc();
    }
#else
#   error PVOID_SIZE not equal INT_SIZE or INT_SIZE*2
#endif

}}


