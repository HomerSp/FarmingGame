#include <functionptr.h> 

std::vector<std::pair<std::string, std::type_index>> FunctionPtrHelper::sTypes;

void FunctionPtrHelper::init()
{
    sTypes.push_back(std::make_pair<std::string, std::type_index>("void", typeid(void)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("bool", typeid(bool)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("int8", typeid(int8_t)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("int16", typeid(int16_t)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("int", typeid(int)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("int64", typeid(int64_t)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("uint8", typeid(uint8_t)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("uint16", typeid(uint16_t)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("uint", typeid(uint)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("uint64", typeid(uint64_t)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("float", typeid(float)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("double", typeid(double)));
    sTypes.push_back(std::make_pair<std::string, std::type_index>("string", typeid(std::string)));
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
    sTypes.push_back(std::make_pair<std::string, std::type_index>(std::string(name), std::type_index(type)));
}

std::string FunctionPtrHelper::typeToName(const std::type_index& id, bool c, bool l, bool r)
{
    for (auto i: sTypes) {
        if (i.second == id) {
            std::string ret = "";
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
    mContext.SetArgByte(mCurrent++, val);
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
