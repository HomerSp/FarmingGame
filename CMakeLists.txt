cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(PROJECT "farming")
project(${PROJECT})

set(assets_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(base_DIR ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(src)
add_subdirectory(external)
