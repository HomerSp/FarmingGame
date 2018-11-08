#pragma once

#include <iostream>
#include <string>

#include <angelscript.h>
#include <functionptr.h>

// Class, return type, function name, parameter types ...
#define SCRIPT_FUNC(c, r, n) FunctionPtrHelper::functionString<r>(#n), asMETHOD(c, n)
#define SCRIPT_FUNC_ARGS(c, r, n, ...) FunctionPtrHelper::functionString<r, __VA_ARGS__>(#n), asMETHOD(c, n)

namespace engine {
class ScriptObject {
public:
    ScriptObject();
    virtual ~ScriptObject() = default;

    template<class T, typename... Ts>
    void registerCallback(asIScriptEngine* engine)
    {
        static_assert (std::is_base_of<FunctionPtrCallback, T>::value, "Callback must inherit FunctionPtrCallback!");

        std::string name = T::className();
        FunctionPtrHelper::registerFuncDef<Ts...>(*engine, name, typeid(T));
    }

    void registerContext(asIScriptContext* context);
    void registerObject(asIScriptEngine* engine);

    template<typename T>
    void registerReference(asIScriptEngine* engine)
    {
        FunctionPtrHelper::registerType(className(), typeid(T));
        registerReference(engine, className());
    }

    // Called from AngelScript
    void AddRef();
    void ReleaseRef();

protected:
    virtual std::string className() = 0;
    virtual void registerClass() = 0;

    asIScriptContext& scriptContext();

    void registerClass(size_t size, const asSFuncPtr& construct, const asSFuncPtr& destruct);
    void registerInstance(const std::string &instanceName);
    void registerMethod(const std::string& decl, const asSFuncPtr &funcPointer);
    void registerProperty(const std::string& decl, int offset);
    void registerType(ScriptObject& o);

private:
    void registerReference(asIScriptEngine* engine, const std::string& name);

    asIScriptEngine* mEngine;
    asIScriptContext* mContext;
    int mRefs;
};

struct ScriptCallback : public FunctionPtrCallback {
    static std::string className()
    {
        return "ScriptCallback";
    }
};
}
