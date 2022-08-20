#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

#include <angelscript.h>
#include <functionptr.h>

#include <engine/types.h>

namespace engine {
class Listeners {
public:
    class Listener {
    public:
        Listener(asIScriptFunction* fun, bool oneShot = false);
        virtual ~Listener();

        bool oneShot() const { return mOneShot; }

        bool maybeTrigger(asIScriptContext& ctx);

    protected:
        void call(asIScriptContext& ctx);

        bool canTrigger() const;
        void setCanTrigger(bool b);

    private:
        bool mOneShot;
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

    template<class T>
    static bool maybeTrigger(asIScriptContext& ctx, std::mutex& mutex, std::vector<std::shared_ptr<T>>& vec)
    {
        std::vector<T *> listeners;
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto& i: vec) {
                listeners.emplace_back(i.get());
            }
        }

        std::vector<T*> toremove;
        for (auto* i: listeners) {
            if (i->maybeTrigger(ctx) && i->oneShot()) {
                toremove.emplace_back(i);
            }
        }

        if (!toremove.empty()) {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto* i: toremove) {
                for (auto it = vec.begin(); it != vec.end(); ++it) {
                    if (it->get() == i) {
                        vec.erase(it);
                        break;
                    }
                }
            }
        }

        return !toremove.empty();
    }
};
}