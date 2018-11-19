#pragma once

#include <mutex>

#include <engine/listeners.h>
#include <engine/scriptobject.h>
#include <engine/types.h>

namespace engine {

class Character;
class Map;

class Camera : public ScriptObject {
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

    Camera(uint32_t width, uint32_t height);

    int32_t x();
    int32_t y();

    uint32_t width() const;
    uint32_t height() const;

    void follow(Target* target);
    void moveTo(int dstX, int dstY, asIScriptFunction* fun = nullptr);

    bool contains(Character& character, const Types::Dimension<>& buf);

    bool processAsync(float frameDiff, Map* map);
    void processListeners();

    void setPosition(int x, int y);
    void setTarget(Target* target);
    void setViewport(const Types::Dimension<uint32_t>& d);

    static void registerClass(asIScriptEngine* engine);
    static std::string className();

private:
    Target* mTarget;
    Types::Dimension<uint32_t> mDimen;
    Types::Point<float> mPos;
    Types::Point<float> mTargetPos;
    std::mutex mMovementMutex;

    std::mutex mListenerMutex;
    std::vector<std::shared_ptr<Listeners::MoveListener>> mMoveListeners;
};
}