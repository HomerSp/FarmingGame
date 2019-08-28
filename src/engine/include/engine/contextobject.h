#pragma once

#include <functional>
#include <memory>

namespace engine {

class Context;
class ListenerObject;

class ContextObject {
public:
    ContextObject(std::shared_ptr<Context> &ctx);

    Context& context();
    std::shared_ptr<Context>& contextPtr();

    void connect(ListenerObject& target, std::function<void()> func);
    void trigger(ListenerObject& target);

private:
    std::shared_ptr<Context> mContext;
};
}
