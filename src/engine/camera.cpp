#include <cmath>

#include <engine/camera.h>
#include <engine/logger.h>

using namespace engine;

Camera::Camera(uint32_t width, uint32_t height)
    : mTarget(nullptr)
    , mTargetPos({0.0f, 0.0f})
    , mDimen(Types::Dimension(width, height))
    , mPos({0.0f, 0.0f})
{
}

float Camera::x() const
{
    return mPos.x;
}

float Camera::y() const
{
    return mPos.y;
}

void Camera::follow(Camera::Target* target)
{
    mTarget = target;
    mTargetPos.x = 0;
    mTargetPos.y = 0;
}

void Camera::moveTo(int dstX, int dstY, asIScriptFunction* fun)
{
    if (fun != nullptr) {
        mMoveListeners.push_back(std::make_shared<Listeners::MoveListener>(fun, dstX, dstY));
    }

    mTarget = nullptr;
    mTargetPos.x = dstX;
    mTargetPos.y = dstY;
}

void Camera::process(uint64_t frameDiff, Map* map)
{
    if (mTarget != nullptr && mTargetPos.x == -1 && mTargetPos.y == -1) {
        mPos.x = mTarget->x() - std::floor((mDimen.width / 2) + (mTarget->width() / 2));
        mPos.y = mTarget->y() - std::floor((mDimen.height / 2) + (mTarget->height() / 2));
    } else {
        float targetX = 0;
        float targetY = 0;
        if (mTarget != nullptr) {
            targetX = mTarget->x() - std::floor((mDimen.width / 2) + (mTarget->width() / 2));
            targetY = mTarget->y() - std::floor((mDimen.height / 2) + (mTarget->height() / 2));
        } else {
            targetX = mTargetPos.x;
            targetY = mTargetPos.y;
        }

        if (mPos.x < 0 && mPos.y < 0) {
            mPos.x = targetX;
            mPos.y = targetY;
        }

        float val = 1.0f * (frameDiff / 20.0f);
        if (mPos.x < targetX) {
            if (mPos.x + val >= targetX) {
                mPos.x = targetX;
            } else {
                mPos.x += val;
            }
        } else if (mPos.x > targetX) {
            if (mPos.x - val <= targetX) {
                mPos.x = targetX;
            } else {
                mPos.x -= val;
            }
        }

        if (mPos.y < targetY) {
            if (mPos.y + val >= targetY) {
                mPos.y = targetY;
            } else {
                mPos.y += val;
            }
        } else if (mPos.y > targetY) {
            if (mPos.y - val <= targetY) {
                mPos.y = targetY;
            } else {
                mPos.y -= val;
            }
        }
    }

    if (mPos.x < 0) {
        mPos.x = 0;
    } else if (mPos.x > map->pixelWidth() - mDimen.width) {
        mPos.x = map->pixelWidth() - mDimen.width;
    }

    if (mPos.y < 0) {
        mPos.y = 0;
    } else if (mPos.y > map->pixelHeight() - mDimen.height) {
        mPos.y = map->pixelHeight() - mDimen.height;
    }

    auto it = mMoveListeners.begin();
    while(it != mMoveListeners.end()) {
        if ((*it)->check(scriptContext(), mPos.x, mPos.y)) {
            it = mMoveListeners.erase(it);
        } else {
            it++;
        }
    }
}

void Camera::setPosition(int x, int y)
{
    mTarget = nullptr;
    mTargetPos.x = -1;
    mTargetPos.y = -1;
    mPos.x = x;
    mPos.y = y;
}

void Camera::setTarget(Camera::Target* target)
{
    mTarget = target;
    mTargetPos.x = -1;
    mTargetPos.y = -1;
    if (mTarget != nullptr) {
        mPos.x = -1;
        mPos.y = -1;
    }
}

void Camera::setViewport(const Types::Dimension& d)
{
    mDimen = d;
}

std::string Camera::className()
{
    return "Camera";
}

void Camera::registerClass()
{
    registerMethod(SCRIPT_FUNC(Camera, float, x));
    registerMethod(SCRIPT_FUNC(Camera, float, y));
    registerMethod(SCRIPT_FUNC_ARGS(Camera, void, moveTo, int, int));
    registerMethod(SCRIPT_FUNC_ARGS(Camera, void, moveTo, int, int, ScriptCallback&&));
}
