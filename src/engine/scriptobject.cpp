#include <engine/logger.h>
#include <engine/scriptobject.h>

using namespace engine;

ScriptObject::ScriptObject()
    : mEngine(nullptr)
    , mContext(nullptr)
    , mRefs(0)
{
}

ScriptObject::~ScriptObject()
{
    release();
}

void ScriptObject::registerContext(asIScriptContext* context)
{
    mContext = context;
}

void ScriptObject::registerObject(asIScriptEngine* engine)
{
    mEngine = engine;
    registerClass();
}

void ScriptObject::registerReference(asIScriptEngine* engine)
{
    mEngine = engine;
    mEngine->RegisterObjectType(className().c_str(), 0, asOBJ_REF);
    mEngine->RegisterObjectBehaviour(className().c_str(), asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptObject, AddRef), asCALL_THISCALL);
    mEngine->RegisterObjectBehaviour(className().c_str(), asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptObject, ReleaseRef), asCALL_THISCALL);

    registerClass();
}

void ScriptObject::release()
{
}

asIScriptContext& ScriptObject::scriptContext()
{
    return *mContext;
}

void ScriptObject::registerClass(size_t size, const asSFuncPtr& construct, const asSFuncPtr& destruct)
{
    mEngine->RegisterObjectType(className().c_str(), size, asOBJ_VALUE);
    mEngine->RegisterObjectBehaviour(className().c_str(), asBEHAVE_CONSTRUCT, "void f()", construct, asCALL_CDECL_OBJLAST);
    mEngine->RegisterObjectBehaviour(className().c_str(), asBEHAVE_DESTRUCT, "void f()", destruct, asCALL_CDECL_OBJLAST);
}

void ScriptObject::registerInstance(const std::string &instanceName)
{
    mEngine->RegisterGlobalProperty(std::string(className() + " " + instanceName).c_str(), this);
}

void ScriptObject::registerMethod(const std::string& decl, const asSFuncPtr &funcPointer)
{
    mEngine->RegisterObjectMethod(className().c_str(), decl.c_str(), funcPointer, asCALL_THISCALL);
}

void ScriptObject::registerProperty(const std::string& decl, int offset)
{
    mEngine->RegisterObjectProperty(className().c_str(), decl.c_str(), offset);
}

void ScriptObject::registerType(ScriptObject& o)
{
    o.registerObject(mEngine);
}

void ScriptObject::AddRef()
{
    mRefs++;
}

void ScriptObject::ReleaseRef()
{
    if (mRefs-- <= 0) {
        delete this;
    }
}