set(PROJECT async)
set(PINC ${INCDIR}/async)
set(PSRC ${SRCDIR}/async)

project(${PROJECT})

set(INC
    ${PSRC}/stable.hpp
    ${PINC}/threadUtilizer.hpp
    ${PINC}/threadState.hpp
    ${PINC}/threadPool.hpp
    ${PINC}/scheduler.hpp
)
source_group(include FILES ${INC})

set(INC_IMPL
    ${PSRC}/impl/scheduler.hpp
    ${PSRC}/impl/thread.hpp
    ${PSRC}/impl/threadContainer.hpp
    ${PSRC}/impl/task.hpp
    ${PSRC}/impl/taskContainer.hpp
    ${PSRC}/impl/contextEngine.hpp
)
source_group(include/impl FILES ${INC_IMPL})

set(SRC
	${PSRC}/threadUtilizer.cpp
	${PSRC}/threadPool.cpp
    ${PSRC}/threadState.cpp
    ${PSRC}/scheduler.cpp
)
source_group(src FILES ${SRC})

set(SRC_IMPL
    ${PSRC}/impl/scheduler.cpp
    ${PSRC}/impl/thread.cpp
    ${PSRC}/impl/threadContainer.cpp
    ${PSRC}/impl/task.cpp
    ${PSRC}/impl/taskContainer.cpp
    ${PSRC}/impl/contextEngine.cpp
)
source_group(src/impl FILES ${SRC_IMPL})



include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PSRC})


include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckTypeSize)

check_type_size("void *" PVOID_SIZE)
check_type_size("int" INT_SIZE)

check_include_files (ucontext.h HAVE_UCONTEXT_H)

if(WIN32)
	include(CheckPrototypeDefinition)
	check_prototype_definition(
		CreateFiberEx
		"LPVOID WINAPI CreateFiberEx(SIZE_T dwStackCommitSize, SIZE_T dwStackReserveSize, DWORD dwFlags, LPFIBER_START_ROUTINE lpStartAddress, LPVOID lpParameter);"
		"NULL"
		windows.h
		HAVE_WINFIBER)
endif()

include(FindValgrind)
if(VALGRIND_FOUND)
    set(HAVE_VALGRIND TRUE)
    message(STATUS "use valgrind: ${VALGRIND_PROGRAM}, ${VALGRIND_INCLUDE_DIR}")
    include_directories(${VALGRIND_INCLUDE_DIR})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)










add_library(${PROJECT} STATIC ${INC} ${SRC} ${SRC_PCH} ${INC_IMPL} ${SRC_IMPL})



