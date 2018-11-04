cmake_minimum_required(VERSION 3.1.0)

set(PROJECT "engine")
project(${PROJECT})

include(${engine_INCLUDE_FILE})

include_directories(
	include
)

set(HEADERS
	include/engine/types.h
	include/engine/engine.h
	include/engine/logger.h
	include/engine/frametimer.h
	include/engine/image.h
	include/engine/renderer.h
	include/engine/assetmanager.h
	include/engine/character.h
	include/engine/keys.h
	include/engine/map.h
	include/engine/charset.h
	include/engine/tileset.h
	include/engine/collisionmap.h
)

set(SOURCES
	types.cpp
	engine.cpp
	logger.cpp
	frametimer.cpp
	image.cpp
	renderer.cpp
	assetmanager.cpp
	character.cpp
	keys.cpp
	map.cpp
	charset.cpp
	tileset.cpp
	collisionmap.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_library( ${PROJECT} STATIC
			 ${SOURCES} )