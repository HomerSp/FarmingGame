include_directories(
    ${src_DIR}/jsoncpp/include
    ${src_DIR}/lodepng/src
    ${src_DIR}/engine/include
)

link_directories(${src_DIR}/jsoncpp)
link_directories(${src_DIR}/lodepng)
link_directories(${src_DIR}/engine)

set(engine_LIBRARY engine jsoncpp_lib_static lodepng)
