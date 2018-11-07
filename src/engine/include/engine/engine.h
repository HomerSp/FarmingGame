#pragma once

#include <memory>
#include <string>

#include <angelscript.h>

#include <engine/character.h>
#include <engine/frametimer.h>
#include <engine/keys.h>
#include <engine/map.h>
#include <engine/clock.h>

namespace engine {
class Engine {
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

    void setSize(int width, int height);

protected:
	bool registerScript();

private:
    bool mHasFocus;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mCameraX;
    uint32_t mCameraY;

    engine::FrameTimer mFrameTimer;
    engine::KeyList mDownKeys;

    std::shared_ptr<engine::Clock> mClock;
    std::shared_ptr<engine::Map> mMap;
    std::shared_ptr<engine::Character> mHero;

    asIScriptEngine *mScriptEngine;
    asIScriptContext *mScriptContext;
};
}
