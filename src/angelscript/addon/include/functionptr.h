#pragma once

#include <sstream>
#include <memory>
#include <typeindex>
#include <vector>

#include <angelscript.h>

struct FunctionPtrCallback {
    virtual ~FunctionPtrCallback() = default;
};

template<typename ...Ts>
class FunctionPtr {
public:
    template<typename T2>
    static std::shared_ptr<FunctionPtr<Ts...>> get(asIScriptFunction* fun);

    static void registerFuncDef(asIScriptEngine& engine, const std::string& name, const std::type_index& type);

    const std::string& name() const
    {
        return mName;
    }

    bool call(asIScriptContext& ctx, Ts... args);

    void release();

protected:
    FunctionPtr(const std::string& name, asIScriptFunction* fun);

private:
    asIScriptFunction* mFunction;
    std::string mName;
};

namespace
{
    template<typename ...Ts>
    struct FunctionPtrMaker: public FunctionPtr<Ts...> {
    FunctionPtrMaker(const std::string& name, asIScriptFunction* fun)
        : FunctionPtr<Ts...>(name, fun)
        {
        }
    };
}

class FunctionPtrHelper {
public:
    static void init();

    template<class R, typename ...Ts>
    static std::string functionString(const std::string& name);

    static void registerFuncDef(asIScriptEngine& engine, const std::string& name, const std::type_index& ret, const std::vector<std::type_index>& params);
    static void registerType(const std::string &name, const std::type_index& type);

    static std::string typeToName(const std::type_index& id);

private:
    static std::vector<std::pair<std::string, std::type_index>> sTypes;
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
FunctionPtr<Ts...>::FunctionPtr(const std::string& name, asIScriptFunction* fun)
    : mFunction(fun)
    , mName(name)
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
    expand_type{0, (functionArgs << args, 0)... };

    ctx.Execute();

    return true;
}

template<typename ...Ts>
void FunctionPtr<Ts...>::release()
{
    if(mFunction != nullptr) {
        mFunction->Release();
    }
}

template<typename ...Ts>
template<typename T2>
std::shared_ptr<FunctionPtr<Ts...>> FunctionPtr<Ts...>::get(asIScriptFunction* fun)
{
    std::string name(T2::name());
    return std::make_shared<FunctionPtrMaker<Ts...>>(name, fun);
}

template<typename ...Ts>
void FunctionPtr<Ts...>::registerFuncDef(asIScriptEngine& engine, const std::string& name, const std::type_index& type)
{
    std::vector<std::type_index> types;
    types.insert(types.end(), {typeid(Ts)...});
    
    FunctionPtrHelper::registerType(name, type);
    FunctionPtrHelper::registerFuncDef(engine, name, typeid(void), types);
}

template<class R, typename ...Ts>
std::string FunctionPtrHelper::functionString(const std::string& name)
{
    std::vector<std::type_index> types;
    types.insert(types.end(), {typeid(Ts)...});
    
    std::stringstream params;
    params << "(";
    for (auto &i: types) {
        if (params.tellp() > 1) {
            params << ", ";
        }
        params << FunctionPtrHelper::typeToName(i);
    }
    params << ")";

    std::stringstream ret;
    ret << typeToName(typeid(R))
        << " "
        << name
        << params.str();
    
    return ret.str();
}