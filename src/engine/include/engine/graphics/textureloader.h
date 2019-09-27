#pragma once

#include <cstdint>
#include <memory>

namespace engine {
namespace graphics {

class Image;
class Renderer;
class Texture;

class TextureLoader {
public:
    TextureLoader(Renderer& renderer);

    void finish(std::unique_ptr<graphics::Texture>& ret);

    TextureLoader& operator+=(const graphics::Image& img);

private:
    Renderer& mRenderer;
    std::vector<std::unique_ptr<graphics::Image>> mImages;
};
}
}
