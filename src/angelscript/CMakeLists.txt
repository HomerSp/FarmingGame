cmake_minimum_required(VERSION 3.5.0)

set(PROJECT "angelscript_function")
project(${PROJECT})

include_directories(
    addon/include
    ${base_DIR}/external/angelscript/src/sdk/angelscript/include
)

set(HEADERS
    addon/include/functionptr.h
)

set(SOURCES
    addon/functionptr.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library( ${PROJECT} STATIC
             ${SOURCES} )