#include <thread>

#include <engine/listeners.h>
#include <engine/logger.h>
#include <engine/scriptobject.h>

using namespace engine;

Listeners::Listener::Listener(asIScriptFunction* fun)
    : mCanTrigger(false)
{
    mFunction = FunctionPtrHelper::get<ScriptCallback>(fun);
}

Listeners::Listener::~Listener()
{
    if (!mFunction) {
        return;
    }

    mFunction->release();
}

void Listeners::Listener::call(asIScriptContext& ctx)
{
    if (!mFunction) {
        return;
    }
    
    mFunction->call(ctx);
}

bool Listeners::Listener::maybeTrigger(asIScriptContext& ctx)
{
    if (!mCanTrigger) {
        return false;
    }

    mCanTrigger = false;
    call(ctx);
    return true;
}

bool Listeners::Listener::canTrigger() const
{
    return mCanTrigger;
}

void Listeners::Listener::setCanTrigger(bool b)
{
    mCanTrigger = b;
}

Listeners::MoveListener::MoveListener(asIScriptFunction* fun, int x, int y)
    : Listener(fun)
    , mTarget(x, y)
{
}

bool Listeners::MoveListener::check(int x, int y, bool canReachX, bool canReachY)
{
    if (canTrigger() || (canReachX && x != mTarget.x) || (canReachY && y != mTarget.y)) {
        return false;
    }

    if (!canReachX && !canReachY) {
        Logger::warning() << "MoveListener can't reach target" << mTarget.x << mTarget.y;
    }

    setCanTrigger(true);
    return true;
}
