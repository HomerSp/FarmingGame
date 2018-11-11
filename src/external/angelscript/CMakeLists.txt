cmake_minimum_required(VERSION 3.1.0)

add_subdirectory(src/sdk/angelscript/projects/cmake)
target_compile_options(angelscript PUBLIC -fno-strict-aliasing -Wno-unused-but-set-variable -Wno-unused-variable)

set(PROJECT "angelscript_addon")
project(${PROJECT})

include_directories(
    src/sdk/angelscript/include
)

set(HEADERS
    src/sdk/add_on/scripthandle/scripthandle.h
    src/sdk/add_on/scriptbuilder/scriptbuilder.h
    src/sdk/add_on/scriptstdstring/scriptstdstring.h
)

set(SOURCES
    src/sdk/add_on/scripthandle/scripthandle.cpp
    src/sdk/add_on/scriptbuilder/scriptbuilder.cpp
    src/sdk/add_on/scriptstdstring/scriptstdstring.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library( ${PROJECT} STATIC
             ${SOURCES} )

target_compile_options(angelscript_addon PUBLIC -fno-strict-aliasing -Wno-unused-but-set-variable -Wno-unused-variable)