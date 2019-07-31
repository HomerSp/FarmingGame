#include <engine/context.h>

using namespace engine; 

Context::Context(Engine& engine, std::shared_ptr<script::ScriptEngine>& scriptEngine, const Renderer& renderer)
    : mEngine(engine)
    , mScriptEngine(scriptEngine)
{
    mAssetManager = std::make_unique<AssetManager>(renderer);
    mFontManager = std::make_unique<FontManager>(*this);
}

Engine& Context::engine()
{
    return mEngine;
}

script::ScriptEngine& Context::scriptEngine()
{
    return *mScriptEngine;
}

AssetManager& Context::assetManager()
{
    return *mAssetManager;
}

FontManager& Context::fontManager()
{
    return *mFontManager;
}

ContextObject::ContextObject(std::shared_ptr<Context> &ctx)
    : mContext(ctx)
{

}

Context& ContextObject::context()
{
    return *mContext;
}

std::shared_ptr<Context>& ContextObject::contextPtr()
{
    return mContext;
}
