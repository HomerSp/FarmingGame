#pragma once

#include <cstdint>
#include <memory>

namespace engine {
namespace graphics {

class Image;

class TextureWriter {
public:
    TextureWriter(Renderer& renderer);

    void finish(std::unique_ptr<graphics::Texture>& ret);

    TextureWriter& operator+=(std::unique_ptr<graphics::Image> img);

private:
    Renderer& mRenderer;
    std::vector<std::unique_ptr<graphics::Image>> mImages;
};
}
}
