cmake_minimum_required(VERSION 3.1.0)

if(LINT)
	set(CMAKE_CXX_CLANG_TIDY "/usr/bin/clang-tidy-6.0;-checks=*,-fuchsia-default-arguments,-google-build-using-namespace")
endif(LINT)

if(DEBUG)
	set(CMAKE_BUILD_TYPE Debug)
endif(DEBUG)

set(PROJECT "src")
project(${PROJECT})

set(src_DIR ${CMAKE_CURRENT_LIST_DIR})
set(engine_INCLUDE_FILE ${src_DIR}/cmake/engine.cmake)

set(CMAKE_MODULE_PATH "${src_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11 -fstrict-aliasing -pedantic-errors -pedantic")

add_subdirectory(engine)
add_subdirectory(app)
add_subdirectory(editor)
add_subdirectory(assets)
