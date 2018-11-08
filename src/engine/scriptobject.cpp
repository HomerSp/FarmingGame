#include <engine/logger.h>
#include <engine/scriptobject.h>

using namespace engine;

ScriptObject::ScriptObject()
    : mContext(nullptr)
    , mRefs(0)
{
}

void ScriptObject::setContext(asIScriptContext* context)
{
    mContext = context;
}

asIScriptContext& ScriptObject::scriptContext()
{
    return *mContext;
}

void ScriptObject::registerReference(asIScriptEngine* engine, const std::string& name)
{
    engine->RegisterObjectType(name.c_str(), 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptObject, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(name.c_str(), asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptObject, ReleaseRef), asCALL_THISCALL);
}

void ScriptObject::AddRef()
{
    mRefs++;
}

void ScriptObject::ReleaseRef()
{
    mRefs--;
}
