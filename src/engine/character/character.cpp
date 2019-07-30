#include <cmath>
#include <fstream>

#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/character/character.h>
#include <engine/logger.h>
#include <engine/map.h>
#include <engine/renderer.h>

using namespace engine;
using namespace engine::character;

Character::Character(std::shared_ptr<script::ScriptEngine> &engine, std::string id)
    : ScriptObject(engine)
    , mValid(false)
    , mID(std::move(id))
    , mName("")
    , mCharsetType(Charset::TypeWalk)
    , mFrame(1)
    , mDirectionTurn(0.0f)
    , mDirectionTo(Direction::Down)
    , mDirection(Direction::Down)
    , mSpeed(1.0f)
    , mMap("")
    , mPos(0, 0)
    , mVelocity(0, 0)
    , mFriction(1.0f)
    , mTargetPos(-1, -1)
    , mTargetNodesCurrent(0)
{
    Logger::debug() << "Character" << mID;

    std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::Character, mID);
    Json::Value doc = *docPtr;
    if (!doc.isObject() || !doc.isMember("name") || !doc.isMember("charset")) {
        Logger::critical() << "Invalid JSON data for character" << mID;
        return;
    }

    mName = doc["name"].asString();
    mCharset = std::make_shared<Charset>(doc["charset"].asString());

    mValid = true;
}

const std::string& Character::id() const
{
    return mID;
}

const std::string& Character::map()
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    return mMap;
}

int32_t Character::x()
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    return static_cast<int32_t>(mPos.x);
}

int32_t Character::y()
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
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

Types::Rect<float_t> Character::rect()
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    return {mPos.x, mPos.y, static_cast<float_t>(mCharset->width(mCharsetType)), static_cast<float_t>(mCharset->height(mCharsetType))};
}

void Character::draw(Renderer& renderer, const Types::Point<>& camera)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    int32_t cols = mCharset->columns(mCharsetType);
    int32_t frame = std::floor(mFrame);
    if(frame >= cols) {
        frame = frame + 1 - cols;
    }

    Types::Point<> pos(mPos.x - camera.x, mPos.y - camera.y);
    mCharset->draw(renderer, pos, mCharsetType, mDirection, frame);
}

bool Character::animate(uint64_t frameDiff, bool reset)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    float_t frame = mFrame;
    if (!reset) {
        int32_t cols = mCharset->columns(mCharsetType);
        frame += frameDiff / 200.0f;
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

bool Character::processAsync(uint64_t frameDiff, std::shared_ptr<Map>& map, std::unordered_map<std::string, std::shared_ptr<Character>> *characters, Camera* camera)
{
    bool changed = false;
    float_t posX, posY;
    int32_t targetX = -1, targetY = -1;
    {
        std::lock_guard<std::mutex> lock(mMovementMutex);
        posX = mPos.x;
        posY = mPos.y;

        if (mTargetPos.x != -1 && mTargetPos.y != -1 && mTargetNodes.empty()) {
            Types::Rect<> col = mCharset->collision(mCharsetType);

            mTargetNodes = PathFinding::find(map, Types::Rect<uint32_t>(posX + col.x, posY + col.y, col.width, col.height), mTargetPos);
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
            int32_t x = 0, y = 0;
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

            updateVelocity(mVelocity.x, x, val, x != 0);
            updateVelocity(mVelocity.y, y, val, y != 0);

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

        // Check if we need to change the direction of the character.
        if (mDirectionTo != mDirection && (mVelocity.x != 0.0f || mVelocity.y != 0.0f)) {
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
                }
            }
        }

        // If we have any velocity we need to process that.
        if (mVelocity.x != 0.0f || mVelocity.y != 0.0f) {
            Types::Point<float_t> dst(mVelocity.x * (frameDiff / 5.0f), mVelocity.y * (frameDiff / 5.0f));

            // Check collisions with the map if we have one
            if (map != nullptr) {
                Types::Rect<> col = mCharset->collision(mCharsetType);

                Types::Point<float_t> pos(posX + col.x, posY + col.y);
                Types::Dimension<> size(col.width, col.height);
                map->checkCollision(pos, size, dst, mVelocity.x, mVelocity.y);
            }

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
        changed = std::floor(mPos.x) != std::floor(posX) || std::floor(mPos.y) != std::floor(posY);
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

void Character::velocity(uint64_t frameDiff, float_t x, float_t y)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    float_t val = frameDiff / 200.0f;
    updateVelocity(mVelocity.x, x, val, !mTargetNodes.empty());
    updateVelocity(mVelocity.y, y, val, !mTargetNodes.empty());
}

bool Character::isMoving()
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    return mVelocity.x != 0.0f || mVelocity.y != 0.0f;
}

void Character::moveTo(int32_t x, int32_t y, asIScriptFunction* fun)
{
    if (fun != nullptr) {
        std::lock_guard<std::mutex> lock(mListenerMutex);
        mMoveListeners.push_back(std::make_shared<Listeners::MoveListener>(fun, x, y));
    }

    std::lock_guard<std::mutex> lock(mMovementMutex);
    mTargetPos.x = x;
    mTargetPos.y = y;
}

void Character::turnTo(Direction::Type direction)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    if (mDirection != direction && mDirectionTo != direction) {
        mDirectionTo = direction;
        mDirectionTurn = 0.0f;
    }
}

void Character::setDirection(Direction::Type direction)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    mDirection = mDirectionTo = direction;
    mDirectionTurn = 0.0f;
}

void Character::setSpeed(float_t speed)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    mSpeed = speed;
}

void Character::setFriction(float_t friction)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    mFriction = friction;
}

void Character::setPosition(const std::string& map, float_t x, float_t y)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    mMap = map;
    mPos.x = x;
    mPos.y = y;
}

void Character::setX(float_t x)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    mPos.x = x;
}

void Character::setY(float_t y)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    mPos.y = y;
}

void Character::checkCollision(const Character& other, Types::Point<float_t>& dst)
{
    Types::Rect<> col = mCharset->collision(mCharsetType);
    Types::Quad<float_t> charQuad(mPos.x + col.x, mPos.y + col.y, mPos.x + col.x + col.width, mPos.y + col.y + col.height);

    Types::Rect<> othercol = other.mCharset->collision(other.mCharsetType);
    Types::Quad<float_t> otherQuad(other.mPos.x + othercol.x, other.mPos.y + othercol.y, other.mPos.x + othercol.x + othercol.width, other.mPos.y + othercol.y + othercol.height);

    // Check x collision.
    if (dst.x != 0.0f && ((charQuad.y1 >= otherQuad.y1 && charQuad.y1 < otherQuad.y2) || (charQuad.y2 >= otherQuad.y1 && charQuad.y2 < otherQuad.y2))) {
        float_t d = dst.x - std::floor(dst.x);
        // Moving Left
        if (dst.x < 0.0f) {
            // We may be moving more than one pixel at a time, which can cause us to move through objects
            // if the distance is longer than the collision object.
            for (int32_t i = std::floor(dst.x); i <= 0; i++) {
                if (charQuad.x1 + i + d <= otherQuad.x2 && charQuad.x1 + i + d > otherQuad.x1) {
                    dst.x = 0.0f;
                }
            }
        // Moving Right
        } else if(dst.x > 0.0f) {
            for (int32_t i = std::floor(dst.x); i >= 0; i--) {
                if (charQuad.x2 + i + d >= otherQuad.x1 && charQuad.x2 + i + d < otherQuad.x2) {
                    dst.x = 0.0f;
                }
            }
        }
    }

    // Check y collision.
    if (dst.y != 0.0f && ((charQuad.x1 >= otherQuad.x1 && charQuad.x1 < otherQuad.x2) || (charQuad.x2 >= otherQuad.x1 && charQuad.x2 < otherQuad.x2))) {
        float_t d = dst.y - std::floor(dst.y);
        // Moving Up
        if (dst.y < 0.0f) {
            for (int32_t i = std::floor(dst.y); i <= 0; i++) {
                if (charQuad.y1 + i + d <= otherQuad.y2 && charQuad.y1 + i + d > otherQuad.y1) {
                    dst.y = 0.0f;
                }
            }
            
        // Moving Down
        } else if(dst.y > 0.0f) {
            for (int32_t i = std::floor(dst.y); i >= 0; i--) {
                if (charQuad.y2 + i + d >= otherQuad.y1 && charQuad.y2 + i + d < otherQuad.y2) {
                    dst.y = 0.0f;
                }
            }
        }
    }
}

void Character::updateVelocity(float_t& velocity, float_t direction, float_t val, bool hasTarget)
{
    if (direction != 0) {
        val *= (direction < 0) ? -direction : direction;
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

std::string Character::className()
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
}
