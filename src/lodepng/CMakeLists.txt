cmake_minimum_required(VERSION 3.1.0)

set(PROJECT "lodepng")
project(${PROJECT})

include_directories(
    src
)

set(HEADERS
    src/lodepng.h
)

set(SOURCES
    src/lodepng.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library( ${PROJECT} STATIC
             ${SOURCES} )