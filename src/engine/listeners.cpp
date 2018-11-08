#include <engine/listeners.h>
#include <engine/logger.h>
#include <engine/scriptobject.h>

using namespace engine;

Listeners::Listener::Listener(asIScriptFunction* fun)
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

Listeners::MoveListener::MoveListener(asIScriptFunction* fun, int x, int y)
    : Listener(fun)
    , mTarget(x, y)
{
}

bool Listeners::MoveListener::check(asIScriptContext& ctx, int x, int y)
{
    if (x != mTarget.x || y != mTarget.y) {
        return false;
    }

    call(ctx);
    return true;
}
