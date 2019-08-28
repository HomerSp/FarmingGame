#include <sstream>

#include <functionptr.h>
#include <scripthandle/scripthandle.h>
#include <scriptstdstring/scriptstdstring.h>

#include <engine/logger.h>
#include <engine/script/scriptengine.h>

using namespace engine;
using namespace engine::script;

ScriptEngine::ScriptEngine()
    : mEngine(nullptr)
    , mContext(nullptr)
{
}

ScriptEngine::~ScriptEngine()
{
    if (mContext != nullptr) {
        mContext->Release();
        mContext = nullptr;
    }

    if (mEngine != nullptr) {
        mEngine->ShutDownAndRelease();
        mEngine = nullptr;
    }
}

void scriptMessageCallback(const asSMessageInfo *msg, void *param)
{
    ((void) param);

    std::stringstream stream;
    stream << "["
        << msg->section
        << ":"
        << msg->row
        << ":"
        << msg->col
        << "]";

    Logger::error("Script") << stream.str() << msg->message;
}

bool ScriptEngine::create()
{
    if (mEngine != nullptr) {
        Logger::error("Script") << "Script engine already created, possible error?";
        return false;
    }

    FunctionPtrHelper::init();

    mEngine = asCreateScriptEngine();
    if (mEngine->SetMessageCallback(asFUNCTION(scriptMessageCallback), nullptr, asCALL_CDECL) != 0) {
        Logger::error("Script") << "Could not register message callback";
        return false;
    }

    RegisterStdString(mEngine);
    RegisterScriptHandle(mEngine);

    return true;
}

bool ScriptEngine::createContext()
{
    if (mContext != nullptr) {
        Logger::error("Script") << "Script context already created, possible error?";
        return false;
    }

    mContext = mEngine->CreateContext();

    return true;
}

asIScriptEngine* ScriptEngine::engine()
{
    return mEngine;
}

asIScriptContext* ScriptEngine::context()
{
    return mContext;
}
