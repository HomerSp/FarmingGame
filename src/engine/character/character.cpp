#include <cmath>

#include <json/value.h>

#include <engine/assetmanager.h>
#include <engine/character/character.h>
#include <engine/context.h>
#include <engine/graphics/renderer.h>
#include <engine/graphics/vector.h>
#include <engine/logger.h>
#include <engine/map.h>

using namespace engine;
using namespace engine::character;

Character::Character(std::shared_ptr<Context> &ctx, graphics::Renderer& renderer, std::string id)
    : ScriptObject(ctx)
    , mValid(false)
    , mID(std::move(id))
    , mName("")
    , mCharsetType(Charset::TypeWalk)
    , mFrame(1)
    , mDirectionTurn(0.0f)
    , mDirectionTo(Direction::None)
    , mDirection(Direction::Down)
    , mSpeed(1.0f)
    , mMap("")
    , mPos(0, 0)
    , mVelocity(0, 0)
    , mFriction(1.0f)
    , mTargetPos(-1, -1)
    , mTargetNodesCurrent(0)
{
    Logger::debug(className()) << mID;

    std::unique_ptr<Json::Value> docPtr = context().assetManager().data(AssetManager::Character, mID);
    Json::Value doc = *docPtr;
    if (!doc.isObject() || !doc.isMember("name") || !doc.isMember("charset")) {
        Logger::critical(className()) << "Invalid JSON data for character" << mID;
        return;
    }

    mName = doc["name"].asString();
    mCharset = std::make_shared<Charset>(ctx, doc["charset"].asString());

    mBuffer = renderer.createBuffer(engine::graphics::Vector4D::Size() * 2 + engine::graphics::Vector2D::Size() + sizeof(float_t) * 2);

    auto& img = mCharset->image();
    mTexture = renderer.createTexture(img.width(), img.height(), 1);
    mTexture->setData(img, 0);

    mValid = true;
}

const std::string& Character::id() const
{
    return mID;
}

const std::string& Character::map()
{
    std::shared_lock<std::shared_timed_mutex> lock(mMovementMutex);
    return mMap;
}

int32_t Character::x()
{
    std::shared_lock<std::shared_timed_mutex> lock(mMovementMutex);
    return static_cast<int32_t>(mPos.x);
}

int32_t Character::y()
{
    std::shared_lock<std::shared_timed_mutex> lock(mMovementMutex);
    return static_cast<int32_t>(mPos.y);
}

uint32_t Character::width() const
{
    return mCharset->width(mCharsetType);
}

uint32_t Character::height() const
{
    return mCharset->height(mCharsetType);
}

int32_t Character::bottom()
{
    std::shared_lock<std::shared_timed_mutex> lock(mMovementMutex);
    Types::Rect<> col = mCharset->collision(mCharsetType);
    return mPos.y + col.height;
}

Types::Rect<float_t> Character::rect()
{
    std::shared_lock<std::shared_timed_mutex> lock(mMovementMutex);
    return {mPos.x, mPos.y, static_cast<float_t>(mCharset->width(mCharsetType)), static_cast<float_t>(mCharset->height(mCharsetType))};
}

void Character::drawBuffer(graphics::Renderer& renderer, const Types::Point<>& dst)
{
    renderer.drawTexturesAnim(dst, mTexture.get(), mBuffer.get(), 0, 1);
}

void Character::updateBuffers(graphics::Renderer& renderer, const Map& map)
{
    std::shared_lock<std::shared_timed_mutex> lock(mMovementMutex);
    int32_t cols = mCharset->columns(mCharsetType);
    int32_t frame = std::floor(mFrame);
    if (frame >= cols) {
        frame = frame + 1 - cols;
    }

    Types::Rect<> col = mCharset->collision(mCharsetType);
    Types::Point<> pos(mPos.x - col.x, mPos.y - col.y);

    int32_t height = mCharset->height(mCharsetType);
    float_t zOrder = 1.0f - ((pos.y + height) / static_cast<float_t>(map.pixelHeight()));

    auto writer = mBuffer->writer();
    mCharset->updateBuffer(renderer, pos, mCharsetType, mDirection, frame, writer, 0, zOrder);
    writer.release();
}

bool Character::animate(uint64_t frameDiff, bool reset)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
    float_t frame = mFrame;
    if (!reset) {
        int32_t cols = mCharset->columns(mCharsetType);
        frame += (frameDiff / 200.0f) * (mSpeed);
        if (frame >= cols + cols - 2) {
            frame = 0;
        }
    } else {
        frame = 1.0f;
    }

    bool changed = std::floor(mFrame) != std::floor(frame);
    mFrame = frame;
    return changed;
}

bool Character::processAsync(uint64_t frameDiff, const Map& map, std::unordered_map<std::string, std::shared_ptr<Character>> *characters, Camera* camera)
{
    bool changed = false;
    float_t posX, posY;
    int32_t targetX = -1, targetY = -1;
    {
        std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
        posX = mPos.x;
        posY = mPos.y;

        if (mTargetPos.x != -1 && mTargetPos.y != -1 && mTargetNodes.empty()) {
            Types::Rect<> col = mCharset->collision(mCharsetType);

            mTargetNodes = PathFinding::find(map, Types::Rect<uint32_t>(posX, posY, col.width, col.height), mTargetPos);
            mTargetNodesCurrent = 1;

            // Did we actually find a path to the destination?
            if (mTargetNodesCurrent < mTargetNodes.size()) {
                mTargetNodesCurrent++;
            } else {
                mTargetPos.x = mTargetPos.y = -1;
                mTargetNodesCurrent = 0;
                mTargetNodes.clear();
            }
        }

        if (mTargetNodesCurrent < mTargetNodes.size()) {
            targetX = mTargetNodes.at(mTargetNodesCurrent).x;
            targetY = mTargetNodes.at(mTargetNodesCurrent).y;
        }

        // Do we have a target? Process it.
        if (targetX != -1 || targetY != -1) {
            float_t val = frameDiff / 200.0f;
            int8_t x = 0, y = 0;
            if (targetX != -1) {
                if (targetX < posX) {
                    x = -1;
                } else if(targetX > posX) {
                    x = 1;
                }
            }

            if (targetY != -1) {
                if (targetY < posY) {
                    y = -1;
                } else if (targetY > posY) {
                    y = 1;
                }
            }

            updateVelocity(mVelocity.x, mVelocity.y, x, val, x != 0);
            updateVelocity(mVelocity.y, mVelocity.x, y, val, y != 0);

            if (mDirectionTo == Direction::None) {
                if (x < 0) {
                    mDirectionTo = Direction::Left;
                } else if(x > 0) {
                    mDirectionTo = Direction::Right;
                }

                if (y < 0) {
                    mDirectionTo = Direction::Up;
                } else if(y > 0) {
                    mDirectionTo = Direction::Down;
                }
            }
        }

        // Check if we need to change the direction of the character.
        if (mDirectionTo != Direction::None) {
            mDirectionTurn += frameDiff / 50.0f;
            if (mDirectionTurn >= 1.0f) {
                mDirectionTurn = 0.0f;

                if (mDirection == Direction::Up && mDirectionTo == Direction::Down) {
                    mDirection = Direction::Left;
                } else if (mDirection == Direction::Down && mDirectionTo == Direction::Up) {
                    mDirection = Direction::Right;
                } else if (mDirection == Direction::Left && mDirectionTo == Direction::Right) {
                    mDirection = Direction::Up;
                } else if (mDirection == Direction::Right && mDirectionTo == Direction::Left) {
                    mDirection = Direction::Down;
                } else {
                    mDirection = mDirectionTo;
                    mDirectionTo = Direction::None;
                }
            }
        }

        // If we have any velocity we need to process that.
        if (mVelocity.x != 0.0f || mVelocity.y != 0.0f) {
            Types::Point<float_t> dst(mVelocity.x * (frameDiff / 5.0f), mVelocity.y * (frameDiff / 5.0f));

            // Check collisions with the map if we have one
            Types::Rect<> col = mCharset->collision(mCharsetType);

            Types::Point<float_t> pos(posX, posY);
            Types::Dimension<> size(col.width, col.height);
            map.checkCollision(pos, size, dst, mVelocity);

            // Check collisions with other characters.
            if (characters != nullptr && (dst.x != 0.0f || dst.y != 0.0f)) {
                for(auto& i: *characters) {
                    // Don't check collision against ourself
                    if (i.second.get() == this) {
                        continue;
                    }

                    // Skip characters that are outside the visible view
                    if (camera->outsideView(i.second->mPos)) {
                        continue;
                    }

                    checkCollision(*(i.second.get()), dst);
                }
            }

            // Check if we have reached the x target.
            float_t velocityX = mVelocity.x, velocityY = mVelocity.y;
            if (targetX != -1 && (
                    (posX > targetX && posX + dst.x <= targetX) ||
                    (posX < targetX && posX + dst.x >= targetX) ||
                    (posX == targetX)
                )
            ) {
                posX = targetX;
                targetX = -1;
                mVelocity.x = 0;
            } else {
                posX += dst.x;
            }

            // Check if we have reached the y target.
            if (targetY != -1 && (
                    (posY > targetY && posY + dst.y <= targetY) ||
                    (posY < targetY && posY + dst.y >= targetY) ||
                    (posY == targetY)
                )
            ) {
                posY = targetY;
                targetY = -1;
                mVelocity.y = 0;
            } else {
                posY += dst.y;
            }

            if (!mTargetNodes.empty() && targetX == -1 && targetY == -1) {
                if (mTargetNodesCurrent < mTargetNodes.size() - 1) {
                    mTargetNodesCurrent++;
                    targetX = mTargetNodes.at(mTargetNodesCurrent).x;
                    targetY = mTargetNodes.at(mTargetNodesCurrent).y;

                    // Use the saved velocity
                    if ( (targetX < mPos.x && velocityX < 0.0f)
                        || (targetX > mPos.x && velocityX > 0.0f)
                    ) {
                        mVelocity.x = velocityX;
                    }

                    if ( (targetY < mPos.y && velocityY < 0.0f)
                        || (targetY > mPos.y && velocityY > 0.0f)
                    ) {
                        mVelocity.y = velocityY;
                    }
                } else {
                    mTargetPos.x = mTargetPos.y = -1;
                    mTargetNodesCurrent = -1;
                    mTargetNodes.clear();
                }
            }
        }

        // Only set changed if we have actually moved, as this will
        // trigger a repaint.
        changed = static_cast<int32_t>(mPos.x) != static_cast<int32_t>(posX) || static_cast<int32_t>(mPos.y) != static_cast<int32_t>(posY);
        mPos.x = posX;
        mPos.y = posY;
    }

    // Check any listeners we may have set.
    std::lock_guard<std::mutex> lock(mListenerMutex);
    for(auto& i: mMoveListeners) {
        i->check(posX, posY);
    }

    return changed;
}

void Character::processListeners()
{
    std::vector<Listeners::MoveListener *> listeners;
    {
        std::lock_guard<std::mutex> lock(mListenerMutex);
        for (auto& i: mMoveListeners) {
            listeners.emplace_back(i.get());
        }
    }

    auto it = listeners.begin();
    while (it != listeners.end()) {
        if (!(*it)->maybeTrigger(scriptContext())) {
            it = listeners.erase(it);
        } else {
            it++;
        }
    }

    std::lock_guard<std::mutex> lock(mListenerMutex);
    for (auto i: listeners) {
        auto moveIt = mMoveListeners.begin();
        while (moveIt != mMoveListeners.end()) {
            if (i == moveIt->get()) {
                moveIt = mMoveListeners.erase(moveIt);
            } else {
                moveIt++;
            }
        }
    }
}

void Character::velocity(uint64_t frameDiff, int8_t x, int8_t y)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
    auto val = frameDiff / 200.0f;
    updateVelocity(mVelocity.x, mVelocity.y, x, val, !mTargetNodes.empty());
    updateVelocity(mVelocity.y, mVelocity.x, y, val, !mTargetNodes.empty());
}

bool Character::isMoving()
{
    std::shared_lock<std::shared_timed_mutex> lock(mMovementMutex);
    return mVelocity.x != 0.0f || mVelocity.y != 0.0f;
}

void Character::moveTo(int32_t x, int32_t y, asIScriptFunction* fun)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);

    // Clear any current target
    mTargetNodesCurrent = -1;
    mTargetNodes.clear();

    mTargetPos.x = x;
    mTargetPos.y = y;

    if (fun != nullptr) {
        std::lock_guard<std::mutex> lock(mListenerMutex);
        mMoveListeners.push_back(std::make_shared<Listeners::MoveListener>(fun, x, y));
    }
}

void Character::turnToDirection(Direction::Type direction)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
    if (mDirection != direction) {
        mDirectionTo = direction;
        if (mDirectionTo == Direction::None) {
            mDirectionTurn = 0.0f;
        }
    }
}

void Character::turnTo(const std::string& d)
{
    if (d == "down") {
        turnToDirection(Direction::Down);
    } else if (d == "up") {
        turnToDirection(Direction::Up);
    } else if (d == "left") {
        turnToDirection(Direction::Left);
    } else if (d == "right") {
        turnToDirection(Direction::Right);
    } else {
        Logger::error(className()) << "Unknown direction" << d;
    }
}

void Character::setDirection(Direction::Type direction)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
    mDirection = direction;
    mDirectionTo = Direction::None;
    mDirectionTurn = 0.0f;
}

void Character::setSpeed(float_t speed)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
    mSpeed = speed;
}

void Character::setFriction(float_t friction)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
    mFriction = friction;
}

void Character::setPosition(const std::string& map, float_t x, float_t y)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
    mMap = map;
    mPos.x = x;
    mPos.y = y;
}

void Character::setX(float_t x)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
    mPos.x = x;
}

void Character::setY(float_t y)
{
    std::unique_lock<std::shared_timed_mutex> lock(mMovementMutex);
    mPos.y = y;
}

void Character::checkCollision(const Character& other, Types::Point<float_t>& dst)
{
    Types::Rect<> col = mCharset->collision(mCharsetType);
    Types::Rect<> othercol = other.mCharset->collision(other.mCharsetType);
    Types::Rect<float_t> otherRc(other.mPos.x, other.mPos.y, othercol.width, othercol.height);

    if (dst.x != 0.0f) {
        Types::Rect<float_t> charRc(mPos.x + std::min(dst.x, 0.0f), mPos.y, col.width + std::abs(dst.x), col.height);
        if (charRc.intersects(otherRc)) {
            dst.x = (dst.x < 0.0f) ? (otherRc.right() - mPos.x) : (otherRc.left() - (mPos.x + col.width));
        }
    }

    if (dst.y != 0.0f) {
        Types::Rect<float_t> charRc(mPos.x, mPos.y + std::min(dst.y, 0.0f), col.width, col.height + std::abs(dst.y));
        if (charRc.intersects(otherRc)) {
            dst.y = (dst.y < 0.0f) ? (otherRc.bottom() - mPos.y) : (otherRc.top() - (mPos.y + col.height));
        }
    }
}

void Character::updateVelocity(float_t& velocity, float_t otherVelocity, int8_t direction, float_t val, bool hasTarget)
{
    if (direction != 0) {
        if (otherVelocity != 0.0f && velocity == 0.0f) {
            val = std::abs(otherVelocity);
        }

        val *= std::abs(direction);
    }

    // Check if we are changing direction, this will increase the friction.
    if ((direction > 0 && velocity < 0.0f) || (direction < 0 && velocity > 0.0f)) {
        val *= mFriction * 5;
    }

    if (direction > 0) {
        if (velocity + val < mSpeed) {
            velocity += val;
        } else if (velocity - val > mSpeed) {
            velocity -= val;
        } else {
            velocity = mSpeed;
        }
    } else if (direction < 0) {
        if (velocity - val > -mSpeed) {
            velocity -= val;
        } else if (velocity + val < -mSpeed) {
            velocity += val;
        } else {
            velocity = -mSpeed;
        }

    // No longer moving, increase the friction.
    } else if (velocity != 0.0f && !hasTarget) {
        val *= mFriction * 5;
        if (velocity > 0.0f) {
            if (velocity - val > 0.0f) {
                velocity -= val;
            } else {
                velocity = 0.0f;
            }
        } else {
            if (velocity + val < 0.0f) {
                velocity += val;
            } else {
                velocity = 0.0f;
            }
        }
    }
}

const std::string Character::className()
{
    return "Character";
}

void Character::registerClass(asIScriptEngine* engine)
{
    registerReference<Character>(engine);
    REGISTER_FUNC(engine, Character, int32_t, x);
    REGISTER_FUNC(engine, Character, int32_t, y);
    REGISTER_FUNC_ARGS(engine, Character, void, moveTo, int32_t, int32_t);
    REGISTER_FUNC_ARGS(engine, Character, void, moveTo, int32_t, int32_t, script::ScriptCallback&&);
    REGISTER_FUNC_ARGS(engine, Character, void, turnTo, const std::string);
}
