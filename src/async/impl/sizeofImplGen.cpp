#include <iostream>
using namespace std;

#include "async/impl/scheduler.hpp"
#include "async/impl/event.hpp"
#include "async/impl/mutex.hpp"
#include "async/impl/waiter.hpp"
#include "async/impl/synchronizerWaiterNode.hpp"

int main()
{
    cout<<

           "#ifndef _ASYNC_SIZEOFIMPL_HPP_\n"
           "#define _ASYNC_SIZEOFIMPL_HPP_\n"
           "\n"
           "#include <cstddef>\n"
           "\n"
           "namespace async\n"
           "{\n"

           "    template <class T>\n"
           "    struct sizeofImpl\n"
           "    {\n"
           "    };\n"
           "\n"


#define ONE(name) \
           "    namespace impl { class " #name "; } \n" \
           "    template <> struct sizeofImpl<impl::" #name "> { static const std::size_t _value =  " << sizeof(async::impl::name) << ";};\n\n"
           ONE(Scheduler)
           ONE(Event)
           ONE(Mutex)
           ONE(Waiter)
           ONE(SynchronizerWaiterNode)


           "}\n"
           "\n"
           "#endif\n"
;

    return 0;
}
