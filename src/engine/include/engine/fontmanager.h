#pragma once

#include <memory>
#include <unordered_map>

namespace engine {
class FontManager {
public:
    static std::shared_ptr<FontManager> get();

    bool files(std::vector<std::string>& out);
    std::string font(const std::string &type);

protected:
    FontManager();

private:
    static std::shared_ptr<FontManager> sInstance;

    std::unordered_map<std::string, std::string> mFonts;
};
}