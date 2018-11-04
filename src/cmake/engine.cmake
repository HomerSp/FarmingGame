find_package(JsonCpp REQUIRED)
find_package(Pngpp REQUIRED)

include_directories(${JsonCpp_INCLUDE_DIRS} ${Pngpp_INCLUDE_DIRS})

include_directories(
    ${src_DIR}/engine/include
)

link_directories(${src_DIR}/engine)

set(engine_LIBRARY engine ${JsonCpp_LIBRARY} ${Pngpp_LIBRARY})
