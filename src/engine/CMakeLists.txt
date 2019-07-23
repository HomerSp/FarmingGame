cmake_minimum_required(VERSION 3.1.0)

set(engine_DIR ${CMAKE_CURRENT_LIST_DIR})

set(PROJECT "engine")
project(${PROJECT})

include(${engine_INCLUDE_FILE})

include_directories(
    include
)

set(HEADERS
	include/engine/assetmanager.h
	include/engine/camera.h
	include/engine/charset.h
	include/engine/clock.h
	include/engine/collisionmap.h
	include/engine/engine.h
	include/engine/frametimer.h
    include/engine/fontmanager.h
    include/engine/hud.h
	include/engine/image.h
    include/engine/item.h
	include/engine/keys.h
	include/engine/listeners.h
	include/engine/logger.h
	include/engine/map.h
    include/engine/player.h
    include/engine/renderer.h
    include/engine/screeneffects.h
	include/engine/tileset.h
	include/engine/time.h
	include/engine/types.h
)

set(SOURCES
	assetmanager.cpp
	camera.cpp
	charset.cpp
	clock.cpp
	collisionmap.cpp
	engine.cpp
	frametimer.cpp
    fontmanager.cpp
    hud.cpp
	image.cpp
    item.cpp
	keys.cpp
	listeners.cpp
    logger.cpp
    map.cpp
    player.cpp
    renderer.cpp
    screeneffects.cpp
    tileset.cpp
    time.cpp
    types.cpp   
)

file(GLOB dirs */dir.cmake)

foreach(loop_var ${dirs})
  include(${loop_var})
endforeach(loop_var)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library( ${PROJECT} STATIC
             ${SOURCES} )
