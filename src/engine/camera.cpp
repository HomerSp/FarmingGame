#include <cmath>

#include <engine/camera.h>
#include <engine/character.h>
#include <engine/logger.h>

using namespace engine;

Camera::Camera(uint32_t width, uint32_t height)
    : mTarget(nullptr)
    , mDimen(width, height)
    , mPosX(0.0f)
    , mPosY(0.0f)
    , mTargetPosX(0.0f)
    , mTargetPosY(0.0f)
{
}

float Camera::x() const
{
    return mPosX;
}

float Camera::y() const
{
    return mPosY;
}

int Camera::width() const
{
    return mDimen.width;
}

int Camera::height() const
{
    return mDimen.height;
}

void Camera::follow(Camera::Target const* target)
{
    std::lock_guard<std::mutex> lock(mTargetMutex);
    mTarget = target;
    mTargetPosX = 0;
    mTargetPosY = 0;
}

void Camera::moveTo(int dstX, int dstY, asIScriptFunction* fun)
{
    if (fun != nullptr) {
        std::lock_guard<std::mutex> lock(mMoveMutex);
        mMoveListeners.push_back(std::make_shared<Listeners::MoveListener>(fun, dstX, dstY));
    }

    std::lock_guard<std::mutex> lock(mTargetMutex);
    mTarget = nullptr;
    mTargetPosX = dstX;
    mTargetPosY = dstY;
}

bool Camera::processAsync(float frameDiff, Map* map)
{
    Camera::Target const* target;
    float posX, posY, targetPosX, targetPosY;
    {
        std::lock_guard<std::mutex> lock(mTargetMutex);
        target = mTarget;
    }

    posX = mPosX;
    posY = mPosY;
    targetPosX = mTargetPosX;
    targetPosY = mTargetPosY;

    if (target != nullptr && targetPosX == -1 && targetPosY == -1) {
        posX =  target->x() + std::floor(target->width() / 2) - std::floor(mDimen.width / 2);
        posY = target->y() + std::floor(target->height() * 0.75f) - std::floor(mDimen.height / 2);
    } else {
        float targetX = 0;
        float targetY = 0;
        if (target != nullptr) {
            targetX = target->x() + std::floor(target->width() / 2) - std::floor(mDimen.width / 2);
            targetY = target->y() + std::floor(target->height() * 0.75f) - std::floor(mDimen.height / 2);
        } else {
            targetX = targetPosX;
            targetY = targetPosY;
        }

        if (posX < 0 && posY < 0) {
            posX = targetX;
            posY = targetY;
        }

        float val = frameDiff * 200.0f;
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

        if (posX == targetX && posY == targetY) {
            targetPosX = -1;
            targetPosY = -1;
        }
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

    bool changed = mPosX != posX || mPosY != posY;
    mPosX = posX;
    mPosY = posY;
    mTargetPosX = targetPosX;
    mTargetPosY = targetPosY;

    std::lock_guard<std::mutex> lock(mMoveMutex);
    for(auto &i: mMoveListeners) {
        i->check(mPosX, mPosY, mTargetPosX >= 0, mTargetPosY >= 0);
    }

    return changed;
}

void Camera::processListeners()
{
    std::vector<Listeners::MoveListener *> toRemove;
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

void Camera::setPosition(int x, int y)
{
    std::lock_guard<std::mutex> lock(mTargetMutex);
    mTarget = nullptr;
    mTargetPosX = -1;
    mTargetPosY = -1;
    mPosX = x;
    mPosY = y;
}

void Camera::setTarget(Camera::Target* target)
{
    std::lock_guard<std::mutex> lock(mTargetMutex);
    mTarget = target;
    mTargetPosX = -1;
    mTargetPosY = -1;
    if (mTarget != nullptr) {
        mPosX = -1;
        mPosY = -1;
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
