#pragma once

#include <iostream>
#include <sstream>
#include <memory>
#include <typeindex>
#include <vector>

#include <angelscript.h>

template<typename ...Ts>
class FunctionPtr {
public:
    static std::unique_ptr<FunctionPtr<Ts...>> get(const std::string& name);

    const std::string& name() const
    {
        return mName;
    }

    bool call(asIScriptContext& ctx, Ts... args);

    void registerFuncDef(asIScriptEngine& engine);

    void release();

    FunctionPtr<Ts...>& operator=(asIScriptFunction* fun);

protected:
    FunctionPtr(const std::string& name);

private:
    asIScriptFunction* mFunction;
    std::string mName;
    std::string mFuncDef;
};

namespace
{
    template<typename ...Ts>
    struct FunctionPtrMaker: public FunctionPtr<Ts...> {
    FunctionPtrMaker(const std::string& name)
        : FunctionPtr<Ts...>(name)
        {
        }
    };
}

class FunctionPtrHelper {
public:
    static void registerFuncDef(asIScriptEngine& engine, const std::string& name, const std::type_index& ret, const std::vector<std::type_index>& params);

    static std::string typeToName(const std::type_index& id);
};

class FunctionPtrArgs {
public:
    FunctionPtrArgs(asIScriptContext& ctx);

    FunctionPtrArgs& operator<<(bool val);
    FunctionPtrArgs& operator<<(int8_t val);
    FunctionPtrArgs& operator<<(int16_t val);
    FunctionPtrArgs& operator<<(int val);
    FunctionPtrArgs& operator<<(int64_t val);
    FunctionPtrArgs& operator<<(uint8_t val);
    FunctionPtrArgs& operator<<(uint16_t val);
    FunctionPtrArgs& operator<<(uint val);
    FunctionPtrArgs& operator<<(uint64_t val);
    FunctionPtrArgs& operator<<(float val);
    FunctionPtrArgs& operator<<(double val);
    FunctionPtrArgs& operator<<(std::string val);

private:
    asIScriptContext& mContext;
    int mCurrent;

};

template<typename ...Ts>
FunctionPtr<Ts...>::FunctionPtr(const std::string& name)
    : mFunction(nullptr)
    , mName("cb__" + name)
    , mFuncDef("")
{
    // void CALLBACK(const string &in)
}

template<typename ...Ts>
bool FunctionPtr<Ts...>::call(asIScriptContext& ctx, Ts... args)
{
    if(mFunction == nullptr) {
        return false;
    }

    ctx.Prepare(mFunction);

    FunctionPtrArgs functionArgs(ctx);

    using expand_type = int[];
    expand_type{ (functionArgs << args, 0)... };

    ctx.Execute();

    return true;
}

template<typename ...Ts>
void FunctionPtr<Ts...>::registerFuncDef(asIScriptEngine& engine)
{
    std::vector<std::type_index> types;
    types.insert(types.end(), {typeid(Ts)...});
    
    FunctionPtrHelper::registerFuncDef(engine, mName, std::type_index(typeid(void)), types);
}

template<typename ...Ts>
FunctionPtr<Ts...>& FunctionPtr<Ts...>::operator=(asIScriptFunction* fun)
{
    mFunction = fun;
    return *this;
}

template<typename ...Ts>
void FunctionPtr<Ts...>::release()
{
    if(mFunction != nullptr) {
        mFunction->Release();
    }
}

template<typename ...Ts>
std::unique_ptr<FunctionPtr<Ts...>> FunctionPtr<Ts...>::get(const std::string& name)
{
    return std::make_unique<FunctionPtrMaker<Ts...>>(name);
}
