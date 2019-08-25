#include <engine/listeners.h>
#include <engine/script/scriptobject.h>

using namespace engine;

Listeners::Listener::Listener(asIScriptFunction* fun)
    : mCanTrigger(false)
{
    mFunction = FunctionPtrHelper::get<script::ScriptCallback>(fun);
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

Listeners::MoveListener::MoveListener(asIScriptFunction* fun, int32_t x, int32_t y)
    : Listener(fun)
    , mTarget(x, y)
{
}

bool Listeners::MoveListener::check(int32_t x, int32_t y, bool force)
{
    if (canTrigger() || (!force && x != mTarget.x) || (!force && y != mTarget.y)) {
        return false;
    }

    setCanTrigger(true);
    return true;
}
