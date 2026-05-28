cmake_minimum_required(VERSION 3.5.0)

set(PROJECT "app")
project(${PROJECT})

include(${engine_INCLUDE_FILE})

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)

include_directories(
    include
)

set(HEADERS
    include/ui/sdlbuffer.h
    include/ui/sdltexture.h
    include/ui/sdlrenderer.h
    include/ui/sdlwindow.h
)

set(SOURCES
    main.cpp
    ui/sdlbuffer.cpp
    ui/sdltexture.cpp
    ui/sdlrenderer.cpp
    ui/sdlwindow.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_executable(${PROJECT} ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT}
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    ${engine_LIBRARY}
)
