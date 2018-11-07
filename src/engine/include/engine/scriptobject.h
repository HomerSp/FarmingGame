#pragma once

#include <string>

#include <angelscript.h>
#include <functionptr.h>

namespace engine {
class ScriptObject {
public:
    ScriptObject();
    virtual ~ScriptObject() = default;

    void registerObject(asIScriptEngine* engine, std::string instance = "");
    void registerContext(asIScriptContext* context);

    // Called from AngelScript
    void AddRef();
    void ReleaseRef();

protected:
    virtual std::string className() = 0;
    virtual void registerClass() = 0;

    virtual std::string globalInstance();

    asIScriptContext& scriptContext();

    void registerClass(size_t size, const asSFuncPtr& construct, const asSFuncPtr& destruct);
    void registerMethod(const std::string& decl, const asSFuncPtr &funcPointer);
    void registerProperty(const std::string& decl, int offset);
    void registerType(ScriptObject& o);

    template<typename... Ts>
    void registerCallback(const std::string& name)
    {
        FunctionPtr<Ts...>::registerFuncDef(*mEngine, name);
    }

private:
    asIScriptEngine* mEngine;
    asIScriptContext* mContext;
    int mRefs;
};
}