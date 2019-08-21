#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <angelscript.h>

#include <engine/character/character.h>
#include <engine/camera.h>
#include <engine/clock.h>
#include <engine/frametimer.h>
#include <engine/hud.h>
#include <engine/keys.h>
#include <engine/map.h>
#include <engine/particles.h>
#include <engine/player.h>
#include <engine/screeneffects.h>
#include <engine/script/scriptengine.h>
#include <engine/script/scriptobject.h>
#include <engine/weather.h>

namespace engine {

class Context;

namespace graphics {
class Renderer;
}

class Engine {
public:
    Engine(uint32_t width, uint32_t height, std::shared_ptr<graphics::Renderer> renderer);
    ~Engine();

    int32_t bufferWidth() const;
    int32_t bufferHeight() const;

    bool process();
    void paint();

    void setKeyMap(const std::unordered_map<int32_t, Keys::Type>& keys);
    void setKeyDown(int32_t key);
    void setKeyUp(int32_t key);

    void setFocus(bool focus)
    {
        mHasFocus = focus;
    }

    void setSize(uint32_t width, uint32_t height);

protected:
    void animateAsync();
    void processAsync();

    bool registerScript();
    void registerContext();

private:
    class EngineObject : public script::ScriptObject {
    public:
        EngineObject(std::shared_ptr<Context> &ctx);

        // Scripting
        engine::Camera* camera();
        engine::Clock* clock();
        engine::Player* player();
        engine::character::Character* character(const std::string& id);

        void registerObject();

        static void registerClass(asIScriptEngine* engine);
        static std::string className();
    };

private:
    std::shared_ptr<script::ScriptEngine> mScriptEngine;
    std::shared_ptr<Context> mContext;

    std::shared_ptr<EngineObject> mEngineObject;

    std::atomic<bool> mRunning;
    std::atomic<bool> mNeedRepaint;
    std::atomic<bool> mHasFocus;
    uint32_t mWidth;
    uint32_t mHeight;

    std::mutex mDownKeysMutex;
    engine::KeyList mKeys;

    std::shared_ptr<graphics::Renderer> mRenderer;

    std::unique_ptr<engine::Hud> mHud;
    std::unique_ptr<engine::ScreenEffects> mScreenEffects;
    std::unique_ptr<engine::Camera> mCamera;
    std::unique_ptr<engine::Clock> mClock;
    std::unique_ptr<engine::Map> mMap;
    std::shared_ptr<engine::Player> mPlayer;
    std::unordered_map<std::string, std::shared_ptr<engine::character::Character>> mCharacters;
    std::unique_ptr<engine::Weather> mWeather;

    std::vector<std::shared_ptr<engine::Overlay::LightSource>> mLights;

    bool mEnableThreading;
    std::vector<std::unique_ptr<std::thread>> mThreads;

    FrameTimer mProcessFrameTimer;
    FrameTimer mAnimationFrameTimer, mDrawingTimer;
};
}
