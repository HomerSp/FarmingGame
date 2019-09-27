#include <engine/graphics/image.h>
#include <engine/graphics/renderer.h>
#include <engine/graphics/texture.h>
#include <engine/graphics/textureloader.h>

using namespace engine;
using namespace engine::graphics;

TextureLoader::TextureLoader(Renderer& renderer)
    : mRenderer(renderer)
{
}

void TextureLoader::finish(std::unique_ptr<graphics::Texture>& ret)
{
    uint32_t w = 0, h = 0;
    for (auto& i: mImages) {
        w = std::max(w, i->width());
        h = std::max(h, i->height());
    }

    ret = mRenderer.createTexture(w, h, mImages.size());
    for (uint32_t i = 0; i < mImages.size(); i++) {
        ret->setData(*mImages[i], i);
    }

    mImages.clear();
}

TextureLoader& TextureLoader::operator+=(const graphics::Image& img)
{
    mImages.emplace_back(std::make_unique<graphics::Image>(img));
    return *this;
}
