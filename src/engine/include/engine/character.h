#pragma once

#include <memory>

#include <png++/png.hpp>

#include <engine/camera.h>
#include <engine/charset.h>
#include <engine/image.h>
#include <engine/listeners.h>
#include <engine/map.h>
#include <engine/renderer.h>
#include <engine/scriptobject.h>

namespace engine {
class Character : public Camera::Target, public ScriptObject {
private:
    class MoveListener {

    };
public:
    struct Direction {
        typedef enum {
            Down = 0,
            Left,
            Right,
            Up,
        } Type;
    };

    Character(const std::string& name);

    void animate(uint64_t currentFrame);
    void draw(Renderer& renderer, const Types::Point& camera);

    void process(uint64_t frameDiff, Map* map = nullptr);
    void reset();
    void velocity(uint64_t frameDiff, int8_t x, int8_t y);

    virtual float x() const;
    virtual float y() const;
    virtual int width() const;
    virtual int height() const;

    bool isMoving() const;

    void moveTo(int x, int y, asIScriptFunction* fun = nullptr);
    void turnTo(Direction::Type direction);

    void setDirection(Direction::Type direction);
    void setSpeed(float speed);
    void setFriction(float friction);
    void setX(float x);
    void setY(float y);

    bool operator!() const
    {
        return !mValid;
    }

    void registerClass(asIScriptEngine* engine);
    static std::string className();

protected:
    void updateVelocity(float& velocity, int8_t direction, float val, bool hasTarget);

private:
    bool mValid;
    std::string mName;
    std::shared_ptr<Charset> mCharset;
    Charset::Type mCharsetType;
    int mFrame;
    std::shared_ptr<engine::Image> mPortrait;

    float mDirectionTurn;
    Direction::Type mDirectionTo;
    Direction::Type mDirection;

    float mSpeed;
    Types::PointF mPos;
    Types::PointF mVelocity;
    Types::Point mTarget;
    float mFriction;

    std::vector<std::shared_ptr<Listeners::MoveListener>> mMoveListeners;
};
}
