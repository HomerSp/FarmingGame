cmake_minimum_required(VERSION 3.1.0)

set(PROJECT "external")
project(${PROJECT})

add_subdirectory(angelscript)

SET(JSONCPP_WITH_TESTS OFF CACHE BOOL "Compile JsonCpp without test")
SET(JSONCPP_WITH_TESTS OFF CACHE BOOL "Compile JsonCpp without unittest")
SET(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE BOOL "Compile JsonCpp without pkgconfig")
SET(JSONCPP_WITH_CMAKE_PACKAGE OFF CACHE BOOL "Compile JsonCpp without cmake")
add_subdirectory(jsoncpp)

add_subdirectory(lodepng)