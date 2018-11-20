#include <cmath>

#include <engine/camera.h>
#include <engine/character.h>
#include <engine/logger.h>
#include <engine/map.h>

using namespace engine;

Camera::Camera(uint32_t width, uint32_t height)
    : mTarget(nullptr)
    , mDimen(width, height)
    , mPos(0.0f, 0.0f)
    , mTargetPos(-1, -1)
{
}

int32_t Camera::x()
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    return static_cast<int32_t>(mPos.x);
}

int32_t Camera::y()
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    return static_cast<int32_t>(mPos.y);
}

uint32_t Camera::width() const
{
    return mDimen.width;
}

uint32_t Camera::height() const
{
    return mDimen.height;
}

void Camera::follow(Camera::Target* target)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    mTarget = target;
    mTargetPos.x = 0;
    mTargetPos.y = 0;
}

void Camera::moveTo(int dstX, int dstY, asIScriptFunction* fun)
{
    if (fun != nullptr) {
        std::lock_guard<std::mutex> lock(mListenerMutex);
        mMoveListeners.push_back(std::make_shared<Listeners::MoveListener>(fun, dstX, dstY));
    }

    std::lock_guard<std::mutex> lock(mMovementMutex);
    mTarget = nullptr;
    mTargetPos.x = dstX;
    mTargetPos.y = dstY;
}

bool Camera::contains(Character& character, const Types::Dimension<>& buf)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    return character.x() + character.width() >= mPos.x - buf.width && character.x() < mPos.x + mDimen.width + buf.width
        && character.y() + character.height() >= mPos.y - buf.height && character.y() < mPos.y + mDimen.height + buf.height;
}

bool Camera::processAsync(uint64_t frameDiff, Map* map)
{
    bool forced = false, changed = false;
    float posX, posY;
    {
        std::lock_guard<std::mutex> lock(mMovementMutex);

        posX = mPos.x;
        posY = mPos.y;
        float targetPosX = mTargetPos.x;
        float targetPosY = mTargetPos.y;

        float targetX = 0;
        float targetY = 0;
        if (mTarget != nullptr) {
            targetX =  mTarget->x() + std::floor(mTarget->width() / 2) - std::floor(mDimen.width / 2);
            targetY = mTarget->y() + std::floor(mTarget->height() * 0.75f) - std::floor(mDimen.height / 2);
        } else {
            targetX = targetPosX;
            targetY = targetPosY;
        }

        if (mTarget == nullptr || targetPosX >= 0 || targetPosY >= 0) {
            if (posX < 0 && posY < 0) {
                posX = targetX;
                posY = targetY;
            }

            float val = frameDiff * 0.2f;
            if (posX < targetX) {
                if (posX + val >= targetX) {
                    posX = targetX;
                } else {
                    posX += val;
                }
            } else if (posX > targetX) {
                if (posX - val <= targetX) {
                    posX = targetX;
                } else {
                    posX -= val;
                }
            }

            if (posY < targetY) {
                if (posY + val >= targetY) {
                    posY = targetY;
                } else {
                    posY += val;
                }
            } else if (posY > targetY) {
                if (posY - val <= targetY) {
                    posY = targetY;
                } else {
                    posY -= val;
                }
            }
        } else {
            posX = targetX;
            posY = targetY;
        }

        if (posX < 0) {
            posX = 0;
            targetPosX = -1;
        } else if (posX > map->pixelWidth() - mDimen.width) {
            posX = map->pixelWidth() - mDimen.width;
            targetPosX = -1;
        }

        if (posY < 0) {
            posY = 0;
            targetPosY = -1;
        } else if (posY > map->pixelHeight() - mDimen.height) {
            posY = map->pixelHeight() - mDimen.height;
            targetPosY = -1;
        }

        forced = mTargetPos.x >= 0 && targetPosX < 0 && mTargetPos.y >= 0 && targetPosY < 0;
        if (posX == targetX && posY == targetY) {
            targetPosX = -1;
            targetPosY = -1;
        }

        changed = mPos.x != posX || mPos.y != posY;
        mPos.x = posX;
        mPos.y = posY;
        mTargetPos.x = targetPosX;
        mTargetPos.y = targetPosY;
    }

    std::lock_guard<std::mutex> lock(mListenerMutex);
    for(auto &i: mMoveListeners) {
        i->check(posX, posY, forced);
    }

    return changed;
}

void Camera::processListeners()
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

void Camera::setPosition(int x, int y)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    mTarget = nullptr;
    mTargetPos.x = -1;
    mTargetPos.y = -1;
    mPos.x = x;
    mPos.y = y;
}

void Camera::setTarget(Camera::Target* target)
{
    std::lock_guard<std::mutex> lock(mMovementMutex);
    mTarget = target;
    mTargetPos.x = -1;
    mTargetPos.y = -1;
    if (mTarget != nullptr) {
        mPos.x = -1;
        mPos.y = -1;
    }
}

void Camera::setViewport(const Types::Dimension<uint32_t>& d)
{
    mDimen = d;
}

std::string Camera::className()
{
    return "Camera";
}

void Camera::registerClass(asIScriptEngine* engine)
{
    registerReference<Camera>(engine);
    registerType<Camera::Target>(engine);
    REGISTER_FUNC(engine, Camera, float, x);
    REGISTER_FUNC(engine, Camera, float, y);
    REGISTER_FUNC_ARGS(engine, Camera, void, moveTo, int, int);
    REGISTER_FUNC_ARGS(engine, Camera, void, moveTo, int, int, ScriptCallback&&);
    REGISTER_FUNC_ARGS(engine, Camera, void, follow, Camera::Target&&);
    REGISTER_FUNC_ARGS(engine, Camera, void, follow, const Character&);
}
