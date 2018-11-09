#pragma once

#include <atomic>

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

        bool maybeTrigger(asIScriptContext& ctx);

    protected:
        void call(asIScriptContext& ctx);

        void setCanTrigger(bool b);

    private:
        std::atomic<bool> mCanTrigger;
        std::shared_ptr<FunctionPtr<>> mFunction;
    };

    class MoveListener : public Listener {
    public:
        MoveListener(asIScriptFunction* fun, int x, int y);
        
        bool check(int x, int y);

    private:
        Types::Point<> mTarget;
    };
};
}