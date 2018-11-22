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

        bool canTrigger() const;
        void setCanTrigger(bool b);

    private:
        std::atomic<bool> mCanTrigger;
        std::shared_ptr<FunctionPtr<>> mFunction;
    };

    class MoveListener : public Listener {
    public:
        MoveListener(asIScriptFunction* fun, int32_t x, int32_t y);
        
        bool check(int32_t x, int32_t y, bool force = false);

    private:
        Types::Point<> mTarget;
    };
};
}