#include <cmath>
#include <fstream>

#include <json/json.h>

#include <engine/assetmanager.h>
#include <engine/character.h>
#include <engine/logger.h>

using namespace engine;

Character::Character(const std::string& name)
    : mValid(false)
    , mName("")
    , mCharsetType(Charset::TypeWalk)
    , mFrame(1)
    , mDirectionTurn(0.0f)
    , mDirectionTo(Direction::Down)
    , mDirection(Direction::Down)
    , mSpeed(1.0f)
    , mPosX(0)
    , mPosY(0)
    , mVelocityX(0)
    , mVelocityY(0)
    , mTargetX(-1)
    , mTargetY(-1)
    , mFriction(1.0f)
{
    Logger::debug() << "Character" << name;

    std::shared_ptr<Json::Value> docPtr = AssetManager::get()->data(AssetManager::get()->Character, name);
    Json::Value doc = *docPtr;
    if (!doc.isObject() || !doc.isMember("name") || !doc.isMember("charset")) {
        Logger::critical() << "Invalid JSON data for character" << name;
        return;
    }

    mName = doc["name"].asString();
    mCharset = std::make_shared<Charset>(doc["charset"].asString());

    mValid = true;
}

float Character::x() const
{
    return mPosX;
}

float Character::y() const
{
    return mPosY;
}

int Character::width() const
{
    return mCharset->width(mCharsetType);
}

int Character::height() const
{
    return mCharset->height(mCharsetType);
}

void Character::draw(Renderer& renderer, const Types::Point& camera)
{
    Types::Point pos(mPosX - camera.x, mPosY - camera.y);
    mCharset->draw(renderer, pos, mCharsetType, mDirection, mFrame);
}

bool Character::animate(double currentFrame, bool reset)
{
    int frame;
    if (!reset) {
        int cols = mCharset->columns(mCharsetType);
        frame = std::floor(static_cast<uint64_t>(currentFrame * 5) % ((cols + cols - 2)));
        if (frame >= cols) {
            frame = frame + 1 - cols;
        }
    } else {
        frame = 1;
    }

    bool changed = mFrame != frame;
    mFrame = frame;
    return changed;
}

bool Character::processAsync(float frameDiff, Map* map)
{
    float posX = mPosX, posY = mPosY;
    float velocityX = mVelocityX, velocityY = mVelocityY;
    float targetX = mTargetX, targetY = mTargetY;

    if (targetX != -1 || targetY != -1) {
        float val = frameDiff * 5.0f;
        int x = 0, y = 0;
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

        updateVelocity(velocityX, x, val, x != 0);
        updateVelocity(velocityY, y, val, y != 0);

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

    if (mDirectionTo != mDirection && (velocityX != 0.0f || velocityY != 0.0f)) {
        mDirectionTurn.store(mDirectionTurn + (frameDiff * 20.0f));
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
                mDirection.store(mDirectionTo);
            }
        }
    }

    if (velocityX != 0.0f || velocityY != 0.0f) {
        Types::PointF dst(velocityX * (frameDiff * 200.0f), velocityY * (frameDiff * 200.0f));
        if (map != nullptr) {
            Types::PointF pos(posX, posY);
            Types::Dimension size(width(), height());
            map->checkCollision(pos, size, dst, velocityX, velocityY);
        }

        if (targetX != -1 && (
                (posX > targetX && posX + dst.x <= targetX) ||
                (posX < targetX && posX + dst.x >= targetX)
            )
        ) {
            posX = targetX;
            targetX = -1;
            velocityX = 0;
        } else {
            posX += dst.x;
        }

        if (targetY != -1 && (
                (posY > targetY && posY + dst.y <= targetY) ||
                (posY < targetY && posY + dst.y >= targetY)
            )
        ) {
            posY = targetY;
            targetY = -1;
            velocityY = 0;
        } else {
            posY += dst.y;
        }
    }

    bool changed = mPosX != posX || mPosY != posY;
    mPosX = posX;
    mPosY = posY;
    mTargetX = targetX;
    mTargetY = targetY;
    mVelocityX = velocityX;
    mVelocityY = velocityY;

    if (mVelocityX != 0.0f || mVelocityY != 0.0f) {
        std::lock_guard<std::mutex> lock(mMoveMutex);
        for(auto& i: mMoveListeners) {
            i->check(mPosX, mPosY);
        }
    }

    return changed;
}

void Character::processListeners()
{
    std::vector<Listeners::MoveListener*> toRemove;
    for(auto& i: mMoveListeners) {
        if (i->maybeTrigger(scriptContext())) {
            toRemove.emplace_back(i.get());
        }
    }

    std::lock_guard<std::mutex> lock(mMoveMutex);
    auto it = toRemove.begin();
    while (it != toRemove.end()) {
        auto it2 = mMoveListeners.begin();
        while (it2 != mMoveListeners.end()) {
            if (it2->get() == *it) {
                mMoveListeners.erase(it2);
                break;
            }

            it2++;
        }

        it++;
    }
}

void Character::velocity(float frameDiff, int8_t x, int8_t y)
{
    float velocityX = mVelocityX, velocityY = mVelocityY;

    float val = frameDiff * 5.0f;
    updateVelocity(velocityX, x, val, mTargetX != -1);
    updateVelocity(velocityY, y, val, mTargetY != -1);

    mVelocityX = velocityX;
    mVelocityY = velocityY;
}

bool Character::isMoving() const
{
    return mVelocityX != 0.0f || mVelocityY != 0.0f;
}

void Character::moveTo(int x, int y, asIScriptFunction* fun)
{
    if (fun != nullptr) {
        std::lock_guard<std::mutex> lock(mMoveMutex);
        mMoveListeners.push_back(std::make_shared<Listeners::MoveListener>(fun, x, y));
    }

    mTargetX = x;
    mTargetY = y;
}

void Character::turnTo(Direction::Type direction)
{
    if (mDirection != direction && mDirectionTo != direction) {
        mDirectionTo = direction;
        mDirectionTurn = 0.0f;
    }
}

void Character::setDirection(Direction::Type direction)
{
    mDirection = mDirectionTo = direction;
    mDirectionTurn = 0.0f;
}

void Character::setSpeed(float speed)
{
    mSpeed = speed;
}

void Character::setFriction(float friction)
{
    mFriction = friction;
}

void Character::setX(float x)
{
    mPosX = x;
}

void Character::setY(float y)
{
    mPosY = y;
}

void Character::updateVelocity(float& velocity, int8_t direction, float val, bool hasTarget)
{
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
    REGISTER_FUNC(engine, Character, float, x);
    REGISTER_FUNC(engine, Character, float, y);
    REGISTER_FUNC_ARGS(engine, Character, void, moveTo, int, int);
    REGISTER_FUNC_ARGS(engine, Character, void, moveTo, int, int, ScriptCallback&&);
}
