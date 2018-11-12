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
    , mPos(0, 0)
    , mVelocity(0, 0)
    , mTarget(-1, -1)
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

void Character::draw(Renderer& renderer, const Types::Point<>& camera)
{
    int cols = mCharset->columns(mCharsetType);
    int frame = std::floor(mFrame);
    if(frame >= cols) {
        frame = frame + 1 - cols;
    }

    Types::Point<> pos(mPos.x - camera.x, mPos.y - camera.y);
    mCharset->draw(renderer, pos, mCharsetType, mDirection, frame);
}

bool Character::animate(float frameDiff, bool reset)
{
    float frame = mFrame;
    if (!reset) {
        int cols = mCharset->columns(mCharsetType);
        frame += frameDiff * 5.0f;
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

bool Character::processAsync(float frameDiff, Map* map, std::vector<std::shared_ptr<Character>> *characters)
{
    float posX = mPos.x, posY = mPos.y;
    float velocityX = mVelocity.x, velocityY = mVelocity.y;
    float targetX = mTarget.x, targetY = mTarget.y;

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
        Types::Point<float> dst(velocityX * (frameDiff * 200.0f), velocityY * (frameDiff * 200.0f));
        if (map != nullptr) {
            Types::Rect<> col = mCharset->collision(mCharsetType);

            Types::Point<float> pos(posX + col.x, posY + col.y);
            Types::Dimension<> size(col.width, col.height);
            map->checkCollision(pos, size, dst, velocityX, velocityY);
        }

        if (characters != nullptr && (dst.x != 0.0f || dst.y != 0.0f)) {
            for(auto& i: *characters) {
                checkCollision(*(i.get()), dst);
            }
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

    bool changed = mPos.x != posX || mPos.y != posY;
    mPos.x = posX;
    mPos.y = posY;
    mTarget.x = targetX;
    mTarget.y = targetY;
    mVelocity.x = velocityX;
    mVelocity.y = velocityY;

    if (mVelocity.x != 0.0f || mVelocity.y != 0.0f) {
        std::lock_guard<std::mutex> lock(mMoveMutex);
        for(auto& i: mMoveListeners) {
            i->check(mPos.x, mPos.y);
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

void Character::velocity(float frameDiff, float x, float y)
{
    float velocityX = mVelocity.x, velocityY = mVelocity.y;

    float val = frameDiff * 5.0f;
    updateVelocity(velocityX, x, val, mTarget.x != -1);
    updateVelocity(velocityY, y, val, mTarget.y != -1);

    mVelocity.x = velocityX;
    mVelocity.y = velocityY;
}

bool Character::isMoving() const
{
    return mVelocity.x != 0.0f || mVelocity.y != 0.0f;
}

void Character::moveTo(int x, int y, asIScriptFunction* fun)
{
    if (fun != nullptr) {
        std::lock_guard<std::mutex> lock(mMoveMutex);
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

void Character::checkCollision(const Character& other, Types::Point<float>& dst)
{
    Types::Rect<> col = mCharset->collision(mCharsetType);
    Types::Rect<> othercol = other.mCharset->collision(other.mCharsetType);

    Types::Quad<float> quad1(mPos.x + col.x, mPos.y + col.y, mPos.x + col.x + col.width, mPos.y + col.y + col.height);
    Types::Quad<float> quad2(other.mPos.x + othercol.x, other.mPos.y + othercol.y, other.mPos.x + othercol.x + othercol.width, other.mPos.y + othercol.y + othercol.height);

    if (dst.x != 0.0f && ((quad1.y1 >= quad2.y1 && quad1.y1 < quad2.y2) || (quad1.y2 >= quad2.y1 && quad1.y2 < quad2.y2))) {
        // Left
        if (dst.x < 0.0f && quad1.x1 + dst.x <= quad2.x2 && quad1.x1 + dst.x > quad2.x1) {
            dst.x = quad2.x2 - quad1.x1;
        // Right
        } else if(dst.x > 0.0f && quad1.x2 + dst.x >= quad2.x1 && quad1.x2 + dst.x < quad2.x2) {
            dst.x = quad2.x1 - quad1.x2;
        }
    }

    if (dst.y != 0.0f && ((quad1.x1 >= quad2.x1 && quad1.x1 < quad2.x2) || (quad1.x2 >= quad2.x1 && quad1.x2 < quad2.x2))) {
        // Top
        if (dst.y < 0.0f && quad1.y1 + dst.y <= quad2.y2 && quad1.y1 + dst.y > quad2.y1) {
            dst.y = quad2.y2 - quad1.y1;
        // Bottom
        } else if(dst.y > 0.0f && quad1.y2 + dst.y >= quad2.y1 && quad1.y2 + dst.y < quad2.y2) {
            dst.y = quad2.y1 - quad1.y2;
        }
    }
}

void Character::updateVelocity(float& velocity, float direction, float val, bool hasTarget)
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
    REGISTER_FUNC(engine, Character, float, x);
    REGISTER_FUNC(engine, Character, float, y);
    REGISTER_FUNC_ARGS(engine, Character, void, moveTo, int, int);
    REGISTER_FUNC_ARGS(engine, Character, void, moveTo, int, int, ScriptCallback&&);
}
