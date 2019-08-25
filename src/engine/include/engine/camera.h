#pragma once

#include <cmath>
#include <mutex>

#include <engine/listeners.h>
#include <engine/script/scriptobject.h>
#include <engine/types.h>

namespace engine {

namespace character {
class Character;
}

namespace script {
class ScriptObject;
}

class Context;
class Map;

class Camera : public engine::script::ScriptObject {
public:
    class Target {
    public:
        virtual ~Target() = default;

        virtual int32_t x() = 0;
        virtual int32_t y() = 0;

        virtual uint32_t width() const
        {
            return 0;
        }

        virtual uint32_t height() const
        {
            return 0;
        }

        static std::string className()
        {
            return "CameraTarget";
        }
    };

    Camera(std::shared_ptr<Context> &ctx, uint32_t width, uint32_t height);

    Types::Rect<float_t> rect();
    Types::Point<float_t> position();

    int32_t x();
    int32_t y();

    uint32_t width();
    uint32_t height();

    void follow(Target* target, asIScriptFunction* fun = nullptr);
    void moveTo(int32_t dstX, int32_t dstY, asIScriptFunction* fun = nullptr);

    bool contains(const Types::Rect<float_t>& rc, const Types::Dimension<>& buf);
    bool outsideView(const Types::Point<float_t>& pos);

    bool processAsync(uint64_t frameDiff, Map* map);
    void processListeners();

    void setPosition(int32_t x, int32_t y);
    void setTarget(Target* target);
    void setViewport(const Types::Dimension<uint32_t>& d);

    static void registerClass(asIScriptEngine* engine);
    static std::string className();

private:
    Target* mTarget;
    Types::Rect<float_t> mRect;
    Types::Point<float_t> mTargetPos;
    std::mutex mMovementMutex;

    std::mutex mListenerMutex;
    std::vector<std::shared_ptr<Listeners::MoveListener>> mMoveListeners;
};
}