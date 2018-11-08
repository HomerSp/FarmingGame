#pragma once

#include <angelscript.h>
#include <functionptr.h>

#include <engine/types.h>

namespace engine {
class Listeners {
public:
    class Listener {
    public:
        Listener(asIScriptFunction* fun);
        virtual ~Listener();

    protected:
        void call(asIScriptContext& ctx);

    private:
        std::shared_ptr<FunctionPtr<>> mFunction;
    };

    class MoveListener : public Listener {
    public:
        MoveListener(asIScriptFunction* fun, int x, int y);
        
        bool check(asIScriptContext& ctx, int x, int y);

    private:
        Types::Point mTarget;
    };
};
}