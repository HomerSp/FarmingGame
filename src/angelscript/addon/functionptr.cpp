#include <iostream>

#include <functionptr.h> 

void FunctionPtrHelper::registerFuncDef(asIScriptEngine& engine, const std::string& name, const std::type_index& ret, const std::vector<std::type_index>& params)
{
    std::stringstream paramStr;
    paramStr << "(";
    for(auto i: params) {
        if(paramStr.tellp() > 1) {
            paramStr << ",";
        }

        paramStr << FunctionPtrHelper::typeToName(i);
    }

    paramStr << ")";

    std::string funcDef = FunctionPtrHelper::typeToName(ret) + " " + name + paramStr.str();
    engine.RegisterFuncdef(funcDef.c_str());
}

std::string FunctionPtrHelper::typeToName(const std::type_index& id)
{
    std::string ret = "";
    if(id == std::type_index(typeid(void))) {
        ret = "void";
    } else if(id == std::type_index(typeid(bool))) {
        ret = "bool";
    } else if(id == std::type_index(typeid(int8_t))) {
        ret = "int8";
    } else if(id == std::type_index(typeid(int16_t))) {
        ret = "int16";
    } else if(id == std::type_index(typeid(int))) {
        ret = "int";
    } else if(id == std::type_index(typeid(int64_t))) {
        ret = "int64";
    } else if(id == std::type_index(typeid(uint8_t))) {
        ret = "uint8";
    } else if(id == std::type_index(typeid(uint16_t))) {
        ret = "uint16";
    } else if(id == std::type_index(typeid(uint))) {
        ret = "uint";
    } else if(id == std::type_index(typeid(uint64_t))) {
        ret = "uint64";
    } else if(id == std::type_index(typeid(float))) {
        ret = "float";
    } else if(id == std::type_index(typeid(double))) {
        ret = "double";
    } else if(id == std::type_index(typeid(std::string))) {
        ret = "string";
    }

    return ret;
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
