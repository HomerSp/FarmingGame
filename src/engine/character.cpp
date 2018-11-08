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
    , mPos({ 0, 0 })
    , mVelocity({ 0, 0 })
    , mTarget({ -1, -1 })
    , mFriction(1.0f)
{
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
    return mPos.x;
}

float Character::y() const
{
    return mPos.y;
}

int Character::width() const
{
    return mCharset->width(mCharsetType);
}

int Character::height() const
{
    return mCharset->height(mCharsetType);
}

void Character::animate(uint64_t currentFrame)
{
    int cols = mCharset->columns(mCharsetType);
    int frame = std::floor(currentFrame % ((cols + cols - 2) * 200) / 200);
    if (frame >= cols) {
        mFrame = frame + 1 - cols;
    } else {
        mFrame = frame;
    }
}

void Character::draw(Renderer& renderer, const Types::Point& camera)
{
    Types::Point pos(mPos.x - camera.x, mPos.y - camera.y);
    mCharset->draw(renderer, pos, mCharsetType, mDirection, mFrame);
}

void Character::process(uint64_t frameDiff, Map* map)
{
    if (mTarget.x != -1 || mTarget.y != -1) {
        float val = 1.0f * (frameDiff / 200.0f);
        int x = 0, y = 0;
        if (mTarget.x != -1) {
            if (mTarget.x < mPos.x) {
                x = -1;
            } else if(mTarget.x > mPos.x) {
                x = 1;
            }
        }

        if (mTarget.y != -1) {
            if (mTarget.y < mPos.y) {
                y = -1;
            } else if (mTarget.y > mPos.y) {
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

    if (mDirectionTo != mDirection && (mVelocity.x != 0.0f || mVelocity.y != 0.0f)) {
        mDirectionTurn += 1.0f * (frameDiff / 100.0f);
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

    if (mVelocity.x != 0.0f || mVelocity.y != 0.0f) {
        Types::PointF dst(mVelocity.x * (frameDiff / 5.0f), mVelocity.y * (frameDiff / 5.0f));
        if (map != nullptr) {
            Types::PointF pos(mPos.x, mPos.y);
            Types::Dimension size(width(), height());
            map->checkCollision(pos, size, dst, mVelocity);
        }

        if (mTarget.x != -1 && (
                (mPos.x > mTarget.x && mPos.x + dst.x <= mTarget.x) ||
                (mPos.x < mTarget.x && mPos.x + dst.x >= mTarget.x)
            )
        ) {
            mPos.x = mTarget.x;
            mTarget.x = -1;
            mVelocity.x = 0;
        } else {
            mPos.x += dst.x;
        }

        if (mTarget.y != -1 && (
                (mPos.y > mTarget.y && mPos.y + dst.y <= mTarget.y) ||
                (mPos.y < mTarget.y && mPos.y + dst.y >= mTarget.y)
            )
        ) {
            mPos.y = mTarget.y;
            mTarget.y = -1;
            mVelocity.y = 0;
        } else {
            mPos.y += dst.y;
        }

        auto it = mMoveListeners.begin();
        while (it != mMoveListeners.end()) {
            if((*it)->check(scriptContext(), mPos.x, mPos.y)) {
                it = mMoveListeners.erase(it);
            } else {
                it++;
            }
        }
    }
}

void Character::reset()
{
    mFrame = 1;
}

void Character::velocity(uint64_t frameDiff, int8_t x, int8_t y)
{
    float val = 1.0f * (frameDiff / 200.0f);
    updateVelocity(mVelocity.x, x, val, mTarget.x != -1);
    updateVelocity(mVelocity.y, y, val, mTarget.y != -1);
}

bool Character::isMoving() const
{
    return mVelocity.x != 0.0f || mVelocity.y != 0.0f;
}

void Character::moveTo(int x, int y, asIScriptFunction* fun)
{
    if (fun != nullptr) {
        mMoveListeners.push_back(std::make_shared<Listeners::MoveListener>(fun, x, y));
    }

    mTarget.x = x;
    mTarget.y = y;
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
    mPos.x = x;
}

void Character::setY(float y)
{
    mPos.y = y;
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

void Character::registerClass()
{
    registerMethod(SCRIPT_FUNC(Character, float, x));
    registerMethod(SCRIPT_FUNC(Character, float, y));
    registerMethod(SCRIPT_FUNC_ARGS(Character, void, moveTo, int, int));
    registerMethod(SCRIPT_FUNC_ARGS(Character, void, moveTo, int, int, ScriptCallback&&));
}
