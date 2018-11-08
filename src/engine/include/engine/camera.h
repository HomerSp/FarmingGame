#pragma once

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
    };

    Camera(uint32_t width, uint32_t height);

    float x() const;
    float y() const;

    void follow(Target* target);
    void moveTo(int dstX, int dstY, asIScriptFunction* fun = nullptr);

    void process(uint64_t frameDiff, Map* map);

    void setPosition(int x, int y);
    void setTarget(Target* target);
    void setViewport(const Types::Dimension& d);

protected:
    virtual std::string className();
    virtual void registerClass();

private:
    Target* mTarget;
    Types::PointF mTargetPos;
    Types::Dimension mDimen;
    Types::PointF mPos;

    std::vector<std::shared_ptr<Listeners::MoveListener>> mMoveListeners;
};
}