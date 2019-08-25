#include <engine/script/scriptengine.h>
#include <engine/script/scriptobject.h>

using namespace engine;
using namespace engine::script;

ScriptObject::ScriptObject(std::shared_ptr<Context> &ctx)
    : ContextObject(ctx)
    , mRefs(0)
{
}

asIScriptContext& ScriptObject::scriptContext()
{
    return *context().scriptEngine().context();
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
