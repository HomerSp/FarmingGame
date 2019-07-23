#pragma once

#include <iostream>
#include <string>

#include <angelscript.h>
#include <functionptr.h>

#define SCRIPT_METHOD(c, m) asSMethodPtr<sizeof(void (c::*)())>::Convert(reinterpret_cast<void (c::*)()>(&c::m))

// Class, return type, function name, parameter types ...
#define REGISTER_FUNC(e, c, r, n) script::ScriptObject::registerMethod<c>(e, FunctionPtrHelper::functionString<r>(#n), SCRIPT_METHOD(c, n))
#define REGISTER_FUNC_ARGS(e, c, r, n, ...) script::ScriptObject::registerMethod<c>(e, FunctionPtrHelper::functionString<r, __VA_ARGS__>(#n), SCRIPT_METHOD(c, n))

namespace engine {
namespace script {

class ScriptEngine;

class ScriptObject {
public:
    ScriptObject(std::shared_ptr<script::ScriptEngine> &engine);
    virtual ~ScriptObject() = default;

    // Called from AngelScript
    void AddRef();
    void ReleaseRef();

    template<class T, typename... Ts>
    static void registerCallback(asIScriptEngine* engine);

    template<typename T>
    static void registerReference(asIScriptEngine* engine);

    template<typename T>
    static void registerClass(asIScriptEngine* engine, const asSFuncPtr& construct, const asSFuncPtr& destruct);

    template<typename T>
    static void registerInstance(asIScriptEngine* engine, const std::string &instanceName, T* ptr);

    template<typename T>
    static void registerMethod(asIScriptEngine* engine, const std::string& decl, const asSFuncPtr &funcPointer);

    template<typename T>
    static void registerProperty(asIScriptEngine* engine, const std::string& decl, int32_t offset);

    template<typename T>
    static void registerType(asIScriptEngine* engine);

protected:
    asIScriptContext& scriptContext();

private:
    static void registerReference(asIScriptEngine* engine, const std::string& name);

    std::shared_ptr<script::ScriptEngine> mEngine;
    int32_t mRefs;
};

struct ScriptCallback : public FunctionPtrCallback {
    static std::string className()
    {
        return "ScriptCallback";
    }
};

template<class T, typename... Ts>
void script::ScriptObject::registerCallback(asIScriptEngine* engine)
{
    static_assert (std::is_base_of<FunctionPtrCallback, T>::value, "Callback must inherit FunctionPtrCallback!");

    std::string name = T::className();
    FunctionPtrHelper::registerFuncDef<Ts...>(*engine, name, typeid(T));
}

template<typename T>
void script::ScriptObject::registerReference(asIScriptEngine* engine)
{
    FunctionPtrHelper::registerType(T::className(), typeid(T));
    registerReference(engine, T::className());
}

template<typename T>
void script::ScriptObject::registerType(asIScriptEngine* engine)
{
    FunctionPtrHelper::registerType(T::className(), typeid(T));
    engine->RegisterObjectType(T::className().c_str(), 0, asOBJ_REF | asOBJ_NOCOUNT);
}

template<typename T>
void script::ScriptObject::registerClass(asIScriptEngine* engine, const asSFuncPtr& construct, const asSFuncPtr& destruct)
{
    engine->RegisterObjectType(T::className().c_str(), sizeof(T), asOBJ_VALUE);
    engine->RegisterObjectBehaviour(T::className().c_str(), asBEHAVE_CONSTRUCT, "void f()", construct, asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour(T::className().c_str(), asBEHAVE_DESTRUCT, "void f()", destruct, asCALL_CDECL_OBJLAST);
}

template<typename T>
void script::ScriptObject::registerInstance(asIScriptEngine* engine, const std::string &instanceName, T* ptr)
{
    engine->RegisterGlobalProperty(std::string(T::className() + " " + instanceName).c_str(), ptr);
}

template<typename T>
void script::ScriptObject::registerMethod(asIScriptEngine* engine, const std::string& decl, const asSFuncPtr &funcPointer)
{
    engine->RegisterObjectMethod(T::className().c_str(), decl.c_str(), funcPointer, asCALL_THISCALL);
}

template<typename T>
void script::ScriptObject::registerProperty(asIScriptEngine* engine, const std::string& decl, int32_t offset)
{
    engine->RegisterObjectProperty(T::className().c_str(), decl.c_str(), offset);
}
}
}
