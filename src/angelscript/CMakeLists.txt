cmake_minimum_required(VERSION 3.1.0)

add_subdirectory(src/sdk/angelscript/projects/cmake)
target_compile_options(angelscript PUBLIC -fno-strict-aliasing -Wno-unused-but-set-variable -Wno-unused-variable)

set(PROJECT "angelscript_addon")
project(${PROJECT})

include_directories(
    addon/include
    src/sdk/angelscript/include
)

set(HEADERS
    addon/include/functionptr.h
    src/sdk/add_on/scriptbuilder/scriptbuilder.h
    src/sdk/add_on/scriptstdstring/scriptstdstring.h
)

set(SOURCES
    addon/functionptr.cpp
    src/sdk/add_on/scriptbuilder/scriptbuilder.cpp
    src/sdk/add_on/scriptstdstring/scriptstdstring.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library( ${PROJECT} STATIC
             ${SOURCES} )