cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11 -fstrict-aliasing -pedantic-errors -pedantic -Wno-deprecated-declarations -Wno-unused-variable")

set(src_DIR ${CMAKE_CURRENT_LIST_DIR})
set(engine_INCLUDE_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/engine.cmake)

add_subdirectory(engine)
add_subdirectory(app)
add_subdirectory(editor)