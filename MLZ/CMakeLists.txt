project (MLZ)

cmake_minimum_required (VERSION 2.8.11)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

find_package (IMLib)

add_executable (MLZ main.cpp)
target_link_libraries (MLZ LINK_PUBLIC ${IMLIB_LIBRARY})
include_directories(${IMLIB_INCLUDE_DIR})

add_executable (GraphBuilder graphBuilder.cpp)
target_link_libraries (GraphBuilder LINK_PUBLIC ${IMLIB_LIBRARY})