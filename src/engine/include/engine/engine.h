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
#include <engine/player.h>
#include <engine/screeneffects.h>
#include <engine/scriptobject.h>

namespace engine {
class Engine : public ScriptObject {
public:
    Engine(uint32_t width, uint32_t height, std::shared_ptr<Renderer> renderer);
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

    // Scripting
    engine::Camera* camera();
    engine::Clock* clock();
    engine::Player* player();
    engine::character::Character* character(const std::string& id);

    static std::string className();

protected:
    void animateAsync();
    void processAsync();

    bool registerScript();
    void registerClass();
    void registerContext();

private:
    class ScriptCreator {
    public:
        ScriptCreator();
        ~ScriptCreator();

        bool create();
        bool createContext();

        asIScriptEngine* engine();
        asIScriptContext* context();

    private:
        asIScriptEngine* mScriptEngine;
        asIScriptContext* mScriptContext;
    };

private:
    std::shared_ptr<ScriptCreator> mScript;

    std::atomic<bool> mRunning;
    std::atomic<bool> mNeedRepaint;
    std::atomic<bool> mHasFocus;
    uint32_t mWidth;
    uint32_t mHeight;

    std::mutex mDownKeysMutex;
    engine::KeyList mKeys;

    std::shared_ptr<Renderer> mRenderer;

    std::shared_ptr<engine::Hud> mHud;
    std::shared_ptr<engine::ScreenEffects> mScreenEffects;
    std::shared_ptr<engine::Camera> mCamera;
    std::shared_ptr<engine::Clock> mClock;
    std::shared_ptr<engine::Map> mMap;
    std::shared_ptr<engine::Player> mPlayer;
    std::vector<std::shared_ptr<engine::character::Character>> mCharacters;

    std::vector<std::shared_ptr<engine::ScreenEffects::LightSource>> mLights;

    bool mEnableThreading;
    std::vector<std::unique_ptr<std::thread>> mThreads;

    FrameTimer mProcessFrameTimer;
    FrameTimer mAnimationFrameTimer;
};
}
