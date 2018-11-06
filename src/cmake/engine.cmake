include_directories(
    ${src_DIR}/angelscript/src/sdk/angelscript/include
    ${src_DIR}/angelscript/src/sdk/add_on
    ${src_DIR}/angelscript/addon/include
    ${src_DIR}/jsoncpp/include
    ${src_DIR}/lodepng/src
    ${src_DIR}/engine/include
)

link_directories(${src_DIR}/jsoncpp)
link_directories(${src_DIR}/lodepng)
link_directories(${src_DIR}/engine)

set(engine_EXTRA_LIBRARY jsoncpp_lib_static lodepng)
set(engine_LIBRARY engine angelscript angelscript_addon ${engine_EXTRA_LIBRARY})
