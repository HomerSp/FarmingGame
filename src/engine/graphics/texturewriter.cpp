#include <engine/graphics/image.h>
#include <engine/graphics/renderer.h>
#include <engine/graphics/texture.h>
#include <engine/graphics/texturewriter.h>

using namespace engine;
using namespace engine::graphics;

TextureWriter::TextureWriter(Renderer& renderer)
    : mRenderer(renderer)
{
}

void TextureWriter::finish(std::unique_ptr<graphics::Texture>& ret)
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
}

TextureWriter& TextureWriter::operator+=(std::unique_ptr<graphics::Image> img)
{
    mImages.emplace_back(std::move(img));
    return *this;
}
