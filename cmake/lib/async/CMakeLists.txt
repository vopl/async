set(PROJECT async)
set(PINC ${INCDIR}/async)
set(PSRC ${SRCDIR}/async)

project(${PROJECT})

set(INC
    ${PSRC}/stable.hpp
    ${PINC}/threadUtilizer.hpp
	${PINC}/threadPool.hpp
    ${PINC}/scheduler.hpp
    ${PSRC}/impl/scheduler.hpp
    ${PSRC}/impl/threadController.hpp
    ${PSRC}/impl/threadContainer.hpp
    ${PSRC}/impl/context.hpp
)
source_group(include FILES ${INC})

set(SRC
	${PSRC}/threadUtilizer.cpp
	${PSRC}/threadPool.cpp
    ${PSRC}/scheduler.cpp
    ${PSRC}/impl/scheduler.cpp
    ${PSRC}/impl/threadController.cpp
    ${PSRC}/impl/threadContainer.cpp
    ${PSRC}/impl/context.cpp
)
source_group(src FILES ${SRC})

set(SRC_IMPL
)
source_group(src/impl FILES ${SRC_IMPL})

set(SRC_IMPL_HDR
)
source_group(src/impl FILES ${SRC_IMPL_HDR})



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










add_library(${PROJECT} STATIC ${INC} ${SRC} ${SRC_PCH} ${SRC_IMPL} ${SRC_IMPL_HDR})



