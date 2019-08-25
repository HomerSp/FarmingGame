#include <engine/context.h>
#include <engine/contextobject.h>

using namespace engine; 

ContextObject::ContextObject(std::shared_ptr<Context> &ctx)
    : mContext(ctx)
{

}

Context& ContextObject::context()
{
    return *mContext;
}

std::shared_ptr<Context>& ContextObject::contextPtr()
{
    return mContext;
}
