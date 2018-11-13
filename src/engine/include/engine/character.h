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
    bool processAsync(float frameDiff, Map* map, std::vector<std::shared_ptr<Character>> *characters = nullptr, Camera* camera = nullptr);
    void processListeners();

    void velocity(float frameDiff, float x, float y);

    virtual int32_t x();
    virtual int32_t y();
    virtual uint32_t width() const;
    virtual uint32_t height() const;

    bool isMoving();

    void moveTo(int32_t x, int32_t y, asIScriptFunction* fun = nullptr);
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

    float mFrame;

    float mDirectionTurn;
    Direction::Type mDirectionTo;
    Direction::Type mDirection;

    float mSpeed;
    Types::Point<float> mPos;
    Types::Point<float> mVelocity;
    Types::Point<int> mTarget;
    float mFriction;
    std::mutex mMovementMutex;

    std::mutex mListenerMutex;
    std::vector<std::shared_ptr<Listeners::MoveListener>> mMoveListeners;
};
}
