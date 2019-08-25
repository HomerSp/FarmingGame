#pragma once

#include <cmath>
#include <memory>
#include <sstream>
#include <typeindex>
#include <vector>

#include <angelscript.h>

struct FunctionPtrCallback {
    virtual ~FunctionPtrCallback() = default;
};

template<typename ...Ts>
class FunctionPtr {
public:
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

    template<typename T2, typename ...Ts>
    static std::shared_ptr<FunctionPtr<Ts...>> get(asIScriptFunction* fun);

    template<class R, typename ...Ts>
    static std::string functionString(const std::string& name);

    template<typename ...T>
    static void registerFuncDef(asIScriptEngine& engine, const std::string& name, const std::type_index& type);
    static void registerFuncDef(asIScriptEngine& engine, const std::string& name, const std::string& ret, const std::vector<std::string>& params);
    static void registerType(const std::string &name, const std::type_index& type);

    template<typename ...Ts>
    static void typesToNames(std::vector<std::string>& out);

    template<class T>
    static std::string typeToName();

private:
    static std::string typeToName(const std::type_index& id, bool c, bool l, bool r);

    static std::vector<std::pair<std::string, std::type_index>> sTypes;
};

class FunctionPtrArgs {
public:
    FunctionPtrArgs(asIScriptContext& ctx);

    FunctionPtrArgs& operator<<(bool val);
    FunctionPtrArgs& operator<<(int8_t val);
    FunctionPtrArgs& operator<<(int16_t val);
    FunctionPtrArgs& operator<<(int32_t val);
    FunctionPtrArgs& operator<<(int64_t val);
    FunctionPtrArgs& operator<<(uint8_t val);
    FunctionPtrArgs& operator<<(uint16_t val);
    FunctionPtrArgs& operator<<(uint32_t val);
    FunctionPtrArgs& operator<<(uint64_t val);
    FunctionPtrArgs& operator<<(float_t val);
    FunctionPtrArgs& operator<<(double_t val);
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

    int dummy[] = {0, (functionArgs << args, 0)... };

    // Avoid unused variable warning
    (void) dummy;

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

template<typename T2, typename ...Ts>
std::shared_ptr<FunctionPtr<Ts...>> FunctionPtrHelper::get(asIScriptFunction* fun)
{
    std::string name(T2::className());
    return std::make_shared<FunctionPtrMaker<Ts...>>(name, fun);
}

template<typename ...Ts>
void FunctionPtrHelper::registerFuncDef(asIScriptEngine& engine, const std::string& name, const std::type_index& type)
{
    std::vector<std::string> types;
    FunctionPtrHelper::typesToNames<Ts...>(types);

    std::string ret = FunctionPtrHelper::typeToName<void>();
    
    FunctionPtrHelper::registerType(name, type);
    FunctionPtrHelper::registerFuncDef(engine, name, ret, types);
}

template<class R, typename ...Ts>
std::string FunctionPtrHelper::functionString(const std::string& name)
{
    std::string r = FunctionPtrHelper::typeToName<R>();

    std::vector<std::string> types;
    FunctionPtrHelper::typesToNames<Ts...>(types);
    
    std::stringstream params;
    params << "(";
    for (auto &i: types) {
        if (params.tellp() > 1) {
            params << ", ";
        }
        params << i;
    }
    params << ")";

    std::stringstream ret;
    ret << r
        << " "
        << name
        << params.str();
    
    return ret.str();
}

template<typename ...Ts>
void FunctionPtrHelper::typesToNames(std::vector<std::string>& out)
{
    std::vector<std::type_index> types;
    types.insert(types.end(), {typeid(Ts)...});

    std::vector<bool> cs, ls, rs;
    cs.insert(cs.end(), {std::is_const<Ts>::value...});
    ls.insert(ls.end(), {std::is_lvalue_reference<Ts>::value...});
    rs.insert(rs.end(), {std::is_rvalue_reference<Ts>::value...});

    for(uint32_t i = 0; i < types.size(); i++) {
        out.push_back(typeToName(types[i], cs[i], ls[i], rs[i]));
    }
}

template<class T>
std::string FunctionPtrHelper::typeToName()
{
    return typeToName(typeid(T), std::is_const<T>::value, std::is_lvalue_reference<T>::value, std::is_rvalue_reference<T>::value);
}
