#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>

#include <engine/camera.h>
#include <engine/charset.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/texture.h>
#include <engine/listeners.h>
#include <engine/pathfinding.h>
#include <engine/script/scriptobject.h>
#include <engine/types.h>

namespace engine {

class Map;

namespace graphics {
class Renderer;
}

namespace character {

class Character : public Camera::Target, public script::ScriptObject {
public:
    struct Direction {
        typedef enum {
            Down,
            Left,
            Right,
            Up,
            None,
        } Type;
    };

    Character(std::shared_ptr<Context> &ctx, graphics::Renderer& renderer, std::string id);

    void drawBuffer(graphics::Renderer& renderer, const Types::Point<>& dst);

    void updateBuffers(graphics::Renderer& renderer, const Map& map);

    bool animate(uint64_t frameDiff, bool reset = false);
    bool processAsync(uint64_t frameDiff, const Map& map, std::unordered_map<std::string, std::shared_ptr<Character>> *characters = nullptr, Camera* camera = nullptr);
    void processListeners();

    void velocity(uint64_t frameDiff, int8_t x, int8_t y);

    const std::string& id() const;

    const std::string& map();
    virtual int32_t x();
    virtual int32_t y();
    virtual uint32_t width() const;
    virtual uint32_t height() const;
    Types::Rect<float_t> rect();
    int32_t bottom();

    bool isMoving();

    void moveTo(int32_t x, int32_t y, asIScriptFunction* fun = nullptr);
    void turnToDirection(Direction::Type direction);
    void turnTo(const std::string& d);

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
    static const std::string className();

protected:
    void checkCollision(const Character& other, Types::Point<float_t>& dst);

    void updateVelocity(float_t& velocity, float_t otherVelocity, int8_t direction, float_t val, bool hasTarget);

private:
    bool mValid;
    std::string mID;
    std::string mName;
    std::shared_ptr<Charset> mCharset;
    Charset::Type mCharsetType;
    std::shared_ptr<graphics::Image> mPortrait;

    float_t mFrame;

    float_t mDirectionTurn;
    Direction::Type mDirectionTo;
    Direction::Type mDirection;

    float_t mSpeed;
    std::string mMap;
    Types::Point<float_t> mPos;
    Types::Point<float_t> mVelocity;
    float_t mFriction;
    Types::Point<int32_t> mTargetPos;
    std::vector<Types::Point<int32_t>> mTargetNodes;
    uint32_t mTargetNodesCurrent;
    std::shared_timed_mutex mMovementMutex;

    std::mutex mListenerMutex;
    std::vector<std::shared_ptr<Listeners::MoveListener>> mMoveListeners;

    std::unique_ptr<graphics::Buffer> mBuffer;
    std::unique_ptr<graphics::Texture> mTexture;
};
}
}
