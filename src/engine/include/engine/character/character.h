#pragma once

#include <memory>
#include <mutex>

#include <engine/camera.h>
#include <engine/charset.h>
#include <engine/listeners.h>
#include <engine/pathfinding.h>
#include <engine/script/scriptobject.h>
#include <engine/types.h>

namespace engine {
class Map;
class Renderer;

namespace character {

class Character : public Camera::Target, public script::ScriptObject {
public:
    struct Direction {
        typedef enum {
            Down = 0,
            Left,
            Right,
            Up,
        } Type;
    };

    Character(std::shared_ptr<script::ScriptEngine> &engine, std::string id);

    void draw(Renderer& renderer, const Types::Point<>& camera);

    bool animate(uint64_t frameDiff, bool reset = false);
    bool processAsync(uint64_t frameDiff, std::shared_ptr<Map>& map, std::unordered_map<std::string, std::shared_ptr<Character>> *characters = nullptr, Camera* camera = nullptr);
    void processListeners();

    void velocity(uint64_t frameDiff, float_t x, float_t y);

    const std::string& id() const;

    const std::string& map();
    virtual int32_t x();
    virtual int32_t y();
    virtual uint32_t width() const;
    virtual uint32_t height() const;
    Types::Rect<float_t> rect();

    bool isMoving();

    void moveTo(int32_t x, int32_t y, asIScriptFunction* fun = nullptr);
    void turnTo(Direction::Type direction);

    void setDirection(Direction::Type direction);
    void setSpeed(float_t speed);
    void setFriction(float_t friction);

    void setPosition(const std::string& map, float_t x, float_t y);
    void setX(float_t x);
    void setY(float_t y);

    bool operator!() const
    {
        return !mValid;
    }

    static void registerClass(asIScriptEngine* engine);
    static std::string className();

protected:
    void checkCollision(const Character& other, Types::Point<float_t>& dst);

    void updateVelocity(float_t& velocity, float_t direction, float_t val, bool hasTarget);

private:
    bool mValid;
    std::string mID;
    std::string mName;
    std::shared_ptr<Charset> mCharset;
    Charset::Type mCharsetType;
    std::shared_ptr<engine::Image> mPortrait;

    float_t mFrame;

    float_t mDirectionTurn;
    Direction::Type mDirectionTo;
    Direction::Type mDirection;

    float_t mSpeed;
    std::string mMap;
    Types::Point<float_t> mPos;
    Types::Point<float_t> mVelocity;
    Types::Point<int32_t> mTarget;
    float_t mFriction;
    Types::Point<int32_t> mTargetNodePos;
    std::vector<Types::Point<int32_t>> mTargetNodes;
    uint32_t mTargetNodesCurrent;
    std::mutex mMovementMutex;

    std::mutex mListenerMutex;
    std::vector<std::shared_ptr<Listeners::MoveListener>> mMoveListeners;
};
}
}
