include_directories(
    ${src_DIR}/external/angelscript/src/sdk/angelscript/include
    ${src_DIR}/external/angelscript/src/sdk/add_on
    ${src_DIR}/angelscript/addon/include
    ${src_DIR}/external/jsoncpp/include
    ${src_DIR}/external/lodepng/src
    ${src_DIR}/engine/include
)

link_directories(${src_DIR}/external/angelscript)
link_directories(${src_DIR}/external/jsoncpp)
link_directories(${src_DIR}/external/lodepng)
link_directories(${src_DIR}/angelscript)
link_directories(${src_DIR}/engine)

set(engine_EXTRA_LIBRARY jsoncpp_lib_static lodepng)
set(engine_LIBRARY engine angelscript angelscript_addon angelscript_function ${engine_EXTRA_LIBRARY})
