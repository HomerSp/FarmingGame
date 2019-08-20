#include <engine/graphics/renderer.h>

using namespace engine;
using namespace engine::graphics;

Context& Renderer::context()
{
    return *mContext;
}

void Renderer::setContext(std::shared_ptr<Context>& ctx)
{
    mContext = ctx;
    initContext();
}
