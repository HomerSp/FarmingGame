#pragma once

#include <iostream>
#include <string>

#include <angelscript.h>
#include <functionptr.h>

namespace engine {
class ScriptObject {
public:
    ScriptObject();
    virtual ~ScriptObject();

    void registerContext(asIScriptContext* context);
    void registerObject(asIScriptEngine* engine);
    void registerReference(asIScriptEngine* engine);

    virtual void release();

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

    template<class T, typename... Ts>
    void registerCallback()
    {
        if (!std::is_base_of<FunctionPtrCallback, T>::value) {
            std::cout << "Not registering\n";
            return;
        }

        std::string name = T::name();
        FunctionPtr<Ts...>::registerFuncDef(*mEngine, name, typeid(T));
    }

private:
    asIScriptEngine* mEngine;
    asIScriptContext* mContext;
    int mRefs;
};
}