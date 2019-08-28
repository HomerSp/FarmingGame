#include <engine/listenerobject.h>

using namespace engine;

void ListenerObject::add(std::function<void()> func)
{
    mCallbacks.emplace_back(std::move(func));
}

void ListenerObject::trigger()
{
    for (auto& c: mCallbacks) {
        c();
    }
}
