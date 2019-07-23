#pragma once

#include <angelscript.h>

namespace engine {
namespace script {
class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    bool create();
    bool createContext();

    asIScriptEngine* engine();
    asIScriptContext* context();

private:
    asIScriptEngine* mEngine;
    asIScriptContext* mContext;
};

}
}
