#include <engine/renderer.h>

using namespace engine;

Context& Renderer::context()
{
    return *mContext;
}

void Renderer::setContext(std::shared_ptr<Context>& ctx)
{
    mContext = ctx;
    initContext();
}
