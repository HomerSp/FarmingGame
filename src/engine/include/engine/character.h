#pragma once

#include <memory>
#include <mutex>

#include <png++/png.hpp>

#include <engine/camera.h>
#include <engine/charset.h>
#include <engine/image.h>
#include <engine/listeners.h>
#include <engine/map.h>
#include <engine/renderer.h>
#include <engine/scriptobject.h>
#include <engine/types.h>

namespace engine {
class Character : public Camera::Target, public ScriptObject {
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

    void draw(Renderer& renderer, const Types::Point<>& camera);

    bool animate(float frameDiff, bool reset = false);
    bool processAsync(float frameDiff, Map* map, std::vector<std::shared_ptr<Character>> *characters = nullptr);
    void processListeners();

    void velocity(float frameDiff, float x, float y);

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

    static void registerClass(asIScriptEngine* engine);
    static std::string className();

protected:
    void checkCollision(const Character& other, Types::Point<float>& dst);

    void updateVelocity(float& velocity, float direction, float val, bool hasTarget);

private:
    bool mValid;
    std::string mName;
    std::shared_ptr<Charset> mCharset;
    Charset::Type mCharsetType;
    std::shared_ptr<engine::Image> mPortrait;

    std::atomic<float> mFrame;

    std::atomic<float> mDirectionTurn;
    std::atomic<Direction::Type> mDirectionTo;
    std::atomic<Direction::Type> mDirection;

    std::atomic<float> mSpeed;
    Types::Point<std::atomic<float>, float> mPos;
    Types::Point<std::atomic<float>, float> mVelocity;
    Types::Point<std::atomic<int>, int> mTarget;
    std::atomic<float> mFriction;

    std::mutex mMoveMutex;
    std::vector<std::shared_ptr<Listeners::MoveListener>> mMoveListeners;
};
}
