#pragma once

#include <memory>

namespace engine {

class Context;

class ContextObject {
public:
    ContextObject(std::shared_ptr<Context> &ctx);

    Context& context();
    std::shared_ptr<Context>& contextPtr();

private:
    std::shared_ptr<Context> mContext;
};
}
