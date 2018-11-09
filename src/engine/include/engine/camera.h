#pragma once

#include <mutex>

#include <engine/listeners.h>
#include <engine/map.h>
#include <engine/scriptobject.h>
#include <engine/types.h>

namespace engine {
class Camera : public ScriptObject {
public:
    class Target {
    public:
        virtual ~Target() = default;

        virtual float x() const = 0;
        virtual float y() const = 0;

        virtual int width() const
        {
            return 0;
        }

        virtual int height() const
        {
            return 0;
        }

        static std::string className()
        {
            return "CameraTarget";
        }
    };

    Camera(uint32_t width, uint32_t height);

    float x() const;
    float y() const;

    void follow(const Target* target);
    void moveTo(int dstX, int dstY, asIScriptFunction* fun = nullptr);

    void processAsync(float frameDiff, Map* map);
    void processListeners();

    void setPosition(int x, int y);
    void setTarget(Target* target);
    void setViewport(const Types::Dimension& d);

    void registerClass(asIScriptEngine* engine);
    static std::string className();

private:
    std::mutex mTargetMutex;
    Target const* mTarget;

    Types::Dimension mDimen;
    std::atomic<float> mPosX, mPosY;
    std::atomic<float> mTargetPosX, mTargetPosY;

    std::mutex mMoveMutex;
    std::vector<std::shared_ptr<Listeners::MoveListener>> mMoveListeners;
};
}