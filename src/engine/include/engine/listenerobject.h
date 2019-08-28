#pragma once

#include <functional>
#include <vector>

#include <engine/contextobject.h>

namespace engine {

class Context;

class ListenerObject {
public:
    ListenerObject() = default;

private:
    friend class ContextObject;

    void add(std::function<void()> func);
    void trigger();

    std::vector<std::function<void()>> mCallbacks;
};
}
