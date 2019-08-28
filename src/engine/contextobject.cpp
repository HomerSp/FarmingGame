#include <engine/context.h>
#include <engine/contextobject.h>
#include <engine/listenerobject.h>

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

void ContextObject::connect(ListenerObject& target, std::function<void()> func)
{
    target.add(std::move(func));
}

void ContextObject::trigger(ListenerObject& target)
{
    target.trigger();
}
