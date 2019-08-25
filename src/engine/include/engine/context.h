#pragma once

#include <memory>

#include <engine/assetmanager.h>
#include <engine/fontmanager.h>

namespace engine {

namespace graphics {
class Renderer;
}

namespace script {
class ScriptEngine;
}

class Engine;

class Context {
public:
    Context(Engine& engine, std::shared_ptr<script::ScriptEngine>& scriptEngine, const graphics::Renderer& renderer);

    Engine& engine();
    script::ScriptEngine& scriptEngine();
    AssetManager& assetManager();
    FontManager& fontManager();

private:
    Engine& mEngine;
    std::shared_ptr<script::ScriptEngine> mScriptEngine;
    std::unique_ptr<AssetManager> mAssetManager;
    std::unique_ptr<FontManager> mFontManager;
};

class ContextObject {
public:
    ContextObject(std::shared_ptr<Context> &ctx);

    Context& context();
    std::shared_ptr<Context>& contextPtr();

private:
    std::shared_ptr<Context> mContext;
};
}
