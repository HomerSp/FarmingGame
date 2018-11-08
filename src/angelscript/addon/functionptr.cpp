#include <functionptr.h> 

std::vector<std::pair<std::string, std::type_index>> FunctionPtrHelper::sTypes;

void FunctionPtrHelper::init()
{
    sTypes.emplace_back(std::make_pair("void", std::type_index(typeid(void))));
    sTypes.emplace_back(std::make_pair("bool", std::type_index(typeid(bool))));
    sTypes.emplace_back(std::make_pair("int8", std::type_index(typeid(int8_t))));
    sTypes.emplace_back(std::make_pair("int16", std::type_index(typeid(int16_t))));
    sTypes.emplace_back(std::make_pair("int", std::type_index(typeid(int))));
    sTypes.emplace_back(std::make_pair("int64", std::type_index(typeid(int64_t))));
    sTypes.emplace_back(std::make_pair("uint8", std::type_index(typeid(uint8_t))));
    sTypes.emplace_back(std::make_pair("uint16", std::type_index(typeid(uint16_t))));
    sTypes.emplace_back(std::make_pair("uint", std::type_index(typeid(uint))));
    sTypes.emplace_back(std::make_pair("uint64", std::type_index(typeid(uint64_t))));
    sTypes.emplace_back(std::make_pair("float", std::type_index(typeid(float))));
    sTypes.emplace_back(std::make_pair("double", std::type_index(typeid(double))));
    sTypes.emplace_back(std::make_pair("string", std::type_index(typeid(std::string))));
}

void FunctionPtrHelper::registerFuncDef(asIScriptEngine& engine, const std::string& name, const std::string& ret, const std::vector<std::string>& params)
{
    std::stringstream paramStr;
    paramStr << "(";
    for(auto &i: params) {
        if(paramStr.tellp() > 1) {
            paramStr << ",";
        }

        paramStr << i;
    }

    paramStr << ")";

    std::string funcDef = ret + " " + name + paramStr.str();
    engine.RegisterFuncdef(funcDef.c_str());
}

void FunctionPtrHelper::registerType(const std::string &name, const std::type_index& type)
{
    sTypes.emplace_back(std::make_pair(std::string(name), std::type_index(type)));
}

std::string FunctionPtrHelper::typeToName(const std::type_index& id, bool c, bool l, bool r)
{
    for (auto i: sTypes) {
        if (i.second == id) {
            std::string ret;
            if (c) {
                ret += "const ";
            }

            ret += i.first;
            if (r) {
                ret += "@";
            } else if (l || c) {
                ret += "&";
            }

            if (c) {
                ret += "in";
            }

            return ret;
        }
    }

    return "";
}

FunctionPtrArgs::FunctionPtrArgs(asIScriptContext& ctx)
    : mContext(ctx)
    , mCurrent(0)
{

}

FunctionPtrArgs& FunctionPtrArgs::operator<<(bool val)
{
    mContext.SetArgByte(mCurrent++, (val) ? 1 : 0);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(int8_t val)
{
    mContext.SetArgByte(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(int16_t val)
{
    mContext.SetArgWord(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(int val)
{
    mContext.SetArgDWord(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(int64_t val)
{
    mContext.SetArgQWord(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(uint8_t val)
{
    mContext.SetArgByte(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(uint16_t val)
{
    mContext.SetArgWord(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(uint val)
{
    mContext.SetArgDWord(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(uint64_t val)
{
    mContext.SetArgQWord(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(float val)
{
    mContext.SetArgFloat(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(double val)
{
    mContext.SetArgDouble(mCurrent++, val);
    return *this;
}

FunctionPtrArgs& FunctionPtrArgs::operator<<(std::string val)
{
    mContext.SetArgObject(mCurrent++, static_cast<void*>(&val));
    return *this;
}
