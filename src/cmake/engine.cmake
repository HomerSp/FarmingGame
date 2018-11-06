find_package(Threads)

include_directories(
    ${src_DIR}/chaiscript/include
    ${src_DIR}/jsoncpp/include
    ${src_DIR}/lodepng/src
    ${src_DIR}/engine/include
)

link_directories(${src_DIR}/jsoncpp)
link_directories(${src_DIR}/lodepng)
link_directories(${src_DIR}/engine)

set(engine_EXTRA_LIBRARY jsoncpp_lib_static lodepng ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
set(engine_LIBRARY engine ${engine_EXTRA_LIBRARY})
