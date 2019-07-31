#pragma once

#include <memory>
#include <unordered_map>

namespace engine {

class Context;

class FontManager {
public:
    FontManager(Context& context);

    bool files(std::vector<std::string>& out);
    std::string font(const std::string &type);

private:
    Context& mContext;
    std::unordered_map<std::string, std::string> mFonts;
};
}