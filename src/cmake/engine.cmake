include_directories(
    ${base_DIR}/external/angelscript/src/sdk/angelscript/include
    ${base_DIR}/external/angelscript/src/sdk/add_on
    ${base_DIR}/external/jsoncpp/include
    ${base_DIR}/external/lodepng/src
    ${src_DIR}/angelscript/addon/include
    ${src_DIR}/engine/include
)

link_directories(${base_DIR}/external/angelscript)
link_directories(${base_DIR}/external/jsoncpp)
link_directories(${base_DIR}/external/lodepng)
link_directories(${src_DIR}/angelscript)
link_directories(${src_DIR}/engine)

set(engine_EXTRA_LIBRARY jsoncpp lodepng)
set(engine_LIBRARY engine angelscript angelscript_addon angelscript_function ${engine_EXTRA_LIBRARY})

if(LINT)
    set(CMAKE_CXX_CLANG_TIDY "/usr/bin/clang-tidy-6.0;-checks=*,-fuchsia-default-arguments,-google-build-using-namespace,-cppcoreguidelines-pro-type-reinterpret-cast,-fuchsia-overloaded-operator;-extra-arg=-Wno-unknown-warning-option")
endif(LINT)

