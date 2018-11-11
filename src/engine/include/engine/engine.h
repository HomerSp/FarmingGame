#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <angelscript.h>

#include <engine/camera.h>
#include <engine/character.h>
#include <engine/clock.h>
#include <engine/frametimer.h>
#include <engine/keys.h>
#include <engine/map.h>
#include <engine/player.h>
#include <engine/scriptobject.h>

namespace engine {
class Engine : public ScriptObject {
public:
    Engine(uint32_t width, uint32_t height);
    ~Engine();

    int bufferWidth() const;
    int bufferHeight() const;

    bool process();
    void paint(Renderer& renderer);

    void setKeyMap(const std::unordered_map<int, Keys::Type>& keys);
    void setKeyDown(int key);
    void setKeyUp(int key);

    void setFocus(bool focus)
    {
        mHasFocus = focus;
    }

    void setSize(uint32_t width, uint32_t height);

    // Scripting
    engine::Camera* camera();
    engine::Clock* clock();
    engine::Player* player();

    static std::string className();

protected:
    void animateAsync();
    void processAsync();

    bool registerScript();
    void registerGeneric();
    void registerClass();
    void registerContext();

private:
    enum {
        FRAMETIMER_CLOCK = 0,
        FRAMETIMER_CAMERA,
        FRAMETIMER_HERO_VELOCITY,
        FRAMETIMER_HERO_MOVEMENT,
        FRAMETIMER_HERO_ANIMATION,
        FRAMETIMER_MAP_ANIMATION,
        FRAMETIMER_FASTFORWARD,
    };

private:
    std::atomic<bool> mRunning;
    std::atomic<bool> mNeedRepaint;
    std::atomic<bool> mHasFocus;
    uint32_t mWidth;
    uint32_t mHeight;

    std::mutex mDownKeysMutex;
    engine::KeyList mDownKeys;

    std::shared_ptr<engine::Camera> mCamera;
    std::shared_ptr<engine::Clock> mClock;
    std::shared_ptr<engine::Map> mMap;
    std::shared_ptr<engine::Player> mPlayer;

    FrameTimer mFrameTimer;
    bool mEnableThreading;
    std::vector<std::unique_ptr<std::thread>> mThreads;

    asIScriptEngine* mScriptEngine;
    asIScriptContext* mScriptContext;
};
}
