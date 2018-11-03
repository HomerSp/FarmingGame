cmake_minimum_required(VERSION 3.1.0)

set(PROJECT "app")
project(${PROJECT})

include(${engine_INCLUDE_FILE})

find_package(Qt5 COMPONENTS Widgets Qml Quick REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS} ${QtQml_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS} ${QtQml_DEFINITIONS} ${${Qt5Quick_DEFINITIONS}})
qt5_add_resources(QT_RESOURCES qml.qrc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(
	include
)

set(HEADERS
	include/ui/qtrenderer.h
	include/ui/mapitemview.h
)

set(SOURCES
    main.cpp
    ui/qtrenderer.cpp
    ui/mapitemview.cpp
)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

add_executable(${PROJECT} ${HEADERS} ${SOURCES} ${QT_RESOURCES})

target_link_libraries(${PROJECT}
    Qt5::Widgets
    Qt5::Qml
    Qt5::Quick
    ${engine_LIBRARY}
    )