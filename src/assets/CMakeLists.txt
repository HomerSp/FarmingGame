cmake_minimum_required(VERSION 3.1.0)

set(PROJECT "assets")
project(${PROJECT})

# JSON Files
set(assets_out_JSON "")
file(GLOB_RECURSE assets_JSON RELATIVE ${assets_ROOT} "${assets_ROOT}/assets/data/*.json")
foreach(json IN ITEMS ${assets_JSON})
    add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/${json}"
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/json_minify "${assets_ROOT}/${json}" "${CMAKE_BINARY_DIR}/${json}"
        COMMENT "Generating data file ${json}..."
        DEPENDS "${assets_ROOT}/${json}"
    )

    set(assets_out_JSON ${assets_out_JSON} "${CMAKE_BINARY_DIR}/${json}")
endforeach(json)

ADD_CUSTOM_TARGET(assets_data ALL
   DEPENDS ${assets_out_JSON}
   DEPENDS json_minify
)

find_package(JsonCpp REQUIRED)
include_directories(${JsonCpp_INCLUDE_DIRS})

set(SOURCES
    utils.cpp
    json_minify.cpp
)

add_executable(json_minify ${SOURCES})
target_link_libraries(json_minify ${JsonCpp_LIBRARY} ${Pngpp_LIBRARY})

# Image Files
set(assets_out_IMAGES "")
file(GLOB_RECURSE assets_IMAGES RELATIVE ${assets_ROOT} "${assets_ROOT}/assets/image/*.png")
foreach(image IN ITEMS ${assets_IMAGES})
    add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/${image}"
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/image_encoder "${assets_ROOT}/${image}" "${CMAKE_BINARY_DIR}/${image}"
        COMMENT "Generating image file ${image}..."
        DEPENDS "${assets_ROOT}/${image}"
    )

    set(assets_out_IMAGES ${assets_out_IMAGES} "${CMAKE_BINARY_DIR}/${image}")
endforeach(image)

ADD_CUSTOM_TARGET(assets_image ALL
   DEPENDS ${assets_out_IMAGES}
   DEPENDS image_encoder
)

find_package(Pngpp REQUIRED)
include_directories(${Pngpp_INCLUDE_DIRS})

set(SOURCES
    utils.cpp
    image_encoder.cpp
)

add_executable(image_encoder ${SOURCES})
target_link_libraries(image_encoder ${Pngpp_LIBRARY})

# Collision Files
set(assets_out_COLLISION "")
file(GLOB_RECURSE assets_COLLISION RELATIVE ${assets_ROOT} "${assets_ROOT}/assets/collision/*.png")
foreach(image IN ITEMS ${assets_COLLISION})
    add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/${image}"
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/collision_encoder "${assets_ROOT}/${image}" "${CMAKE_BINARY_DIR}/${image}"
        COMMENT "Generating collision file ${image}..."
        DEPENDS "${assets_ROOT}/${image}"
    )

    set(assets_out_COLLISION ${assets_out_COLLISION} "${CMAKE_BINARY_DIR}/${image}")
endforeach(image)

ADD_CUSTOM_TARGET(assets_collision ALL
   DEPENDS ${assets_out_COLLISION}
   DEPENDS collision_encoder
)

find_package(Pngpp REQUIRED)
include_directories(${Pngpp_INCLUDE_DIRS})

set(SOURCES
    utils.cpp
    collision_encoder.cpp
)

add_executable(collision_encoder ${SOURCES})
target_link_libraries(collision_encoder ${Pngpp_LIBRARY})