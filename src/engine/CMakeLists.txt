cmake_minimum_required(VERSION 3.1.0)

set(PROJECT "engine")
project(${PROJECT})

include(${engine_INCLUDE_FILE})

include_directories(
    include
)

set(HEADERS
	include/engine/assetmanager.h
	include/engine/character.h
	include/engine/charset.h
	include/engine/clock.h
	include/engine/collisionmap.h
	include/engine/engine.h
	include/engine/frametimer.h
	include/engine/image.h
	include/engine/keys.h
	include/engine/logger.h
	include/engine/map.h
	include/engine/scriptobject.h
	include/engine/renderer.h
	include/engine/tileset.h
	include/engine/types.h
)

set(SOURCES
	assetmanager.cpp
	character.cpp
	charset.cpp
	clock.cpp
	collisionmap.cpp
	engine.cpp
	frametimer.cpp
	image.cpp
	keys.cpp
    logger.cpp
    map.cpp
    renderer.cpp
    scriptobject.cpp
    tileset.cpp
    types.cpp   
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library( ${PROJECT} STATIC
             ${SOURCES} )