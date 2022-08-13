#include <cmath>
#include <string>

#include <lodepng.h>

#include <engine/assetmanager.h>
#include <engine/collisionmap.h>
#include <engine/logger.h>
#include <engine/types.h>

using namespace engine;

CollisionMap::CollisionMap(uint32_t width, uint32_t height)
    : mValid(false)
    , mWidth(width)
    , mHeight(height)
{
    mSolid.resize(mWidth * mHeight, 0U);
    mTransparent.resize(mWidth * mHeight, 0U);
}

CollisionMap::CollisionMap(const std::string& path)
    : CollisionMap()
{
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> data;

    lodepng::load_file(buffer, path);
    if (lodepng::decode(data, mWidth, mHeight, buffer) != 0) {
        return;
    }

    mSolid.resize(mWidth * mHeight, 0U);
    mTransparent.resize(mWidth * mHeight, 0U);

    for (uint32_t y = 0; y < mHeight; y++) {
        for (uint32_t x = 0; x < mWidth; x++) {
            auto d = data[((y * mWidth) + x) * 4];
            if (d < 75 || d > 180) {
                mSolid[(y * mWidth) + x] = d != 255;
            } else {
                mTransparent[(y * mWidth) + x] = true;
            }
        }
    }

    mValid = true;
}

std::pair<bool, bool> CollisionMap::check(const Types::Point<uint32_t>& pos, const Types::Point<>& dst, const Types::Dimension<>& size, Types::Quad<>* diff) const
{
    if (diff != nullptr) {
        diff->left = diff->top = diff->right = diff->bottom = -1;
    }

    std::pair<bool, bool> ret(false, false);

    Types::Quad<> xdiff, ydiff;
    if (checkX(pos, dst.x, size, (diff != nullptr) ? &xdiff : nullptr)) {
        ret.first = true;
        if (diff != nullptr) {
            diff->top = xdiff.top;
            diff->bottom = xdiff.bottom;
            if (dst.x < 0) {
                diff->left = xdiff.left;
            } else {
                diff->right = xdiff.right;
            }
        }
    }

    if (checkY(pos, dst.y, size, (diff != nullptr) ? &ydiff : nullptr)) {
        ret.second = true;
        if (diff != nullptr) {
            diff->left = ydiff.left;
            diff->right = ydiff.right;
            if (dst.y < 0) {
                diff->top = ydiff.top;
            } else {
                diff->bottom = ydiff.bottom;
            }
        }
    }

    return ret;
}

bool CollisionMap::checkX(const Types::Point<uint32_t>& pos, int32_t dst, const Types::Dimension<>& size, Types::Quad<>* diff) const
{
    if (diff != nullptr) {
        diff->left = diff->right = diff->top = diff->bottom = -1;
    }

    auto start = pos.x + (dst >= 0 ? (size.width - 1) : 0);
    bool found = false;
    for (uint32_t cx = 0; cx < std::abs(dst); ++cx) {
        auto cur = start + (dst >= 0 ? (cx + 1) : -(cx + 1));
        for (uint32_t cy = 0; cy < std::max(1, size.height); ++cy) {
            // Check top and bottom simultaneously
            auto ts = solid(cur, pos.y + cy);
            auto bs = solid(cur, pos.y + size.height - cy - 1);
            if (ts || bs) {
                found = true;
                if (diff == nullptr) {
                    return true;
                }

                if (dst < 0 && diff->left == -1) {
                    diff->left = cx;
                } else if (dst > 0 && diff->right == -1) {
                    diff->right = cx;
                }

                if (ts && diff->top == -1) {
                    diff->top = cy;
                }
                if (bs && diff->bottom == -1) {
                    diff->bottom = cy;
                }

                if (diff->top != -1 && diff->bottom != -1) {
                    return found;
                }
            }

            // No need to continue checking if we've reached the centre point
            if (cy == size.height - cy - 1) {
                break;
            }
        }
    }

    return found;
}

bool CollisionMap::checkY(const Types::Point<uint32_t>& pos, int32_t dst, const Types::Dimension<>& size, Types::Quad<>* diff) const
{
    if (diff != nullptr) {
        diff->left = diff->right = diff->top = diff->bottom = -1;
    }

    auto start = pos.y + (dst >= 0 ? (size.height - 1) : 0);
    bool found = false;
    for (uint32_t cy = 0; cy < std::abs(dst); ++cy) {
        auto cur = (start + (dst >= 0 ? (cy + 1) : -(cy + 1)));
        for (uint32_t cx = 0; cx < std::max(1, size.width); ++cx) {
            // Check left and right simultaneously
            auto ls = solid(pos.x + cx, cur);
            auto rs = solid(pos.x + size.width - cx - 1, cur);
            if (ls || rs) {
                found = true;
                if (diff == nullptr) {
                    return true;
                }

                if (dst < 0 && diff->top == -1) {
                    diff->top = cy;
                } else if (dst > 0 && diff->bottom == -1) {
                    diff->bottom = cy;
                }

                if (ls && diff->left == -1) {
                    diff->left = cx;
                }
                if (rs && diff->right == -1) {
                    diff->right = cx;
                }

                if (diff->left != -1 && diff->right != -1) {
                    return found;
                }
            }

            // No need to continue checking if we've reached the centre point
            if (cx == size.width - cx - 1) {
                break;
            }
        }
    }

    return found;
}

bool CollisionMap::solid(const Types::Point<uint32_t>& pos, const Types::Dimension<>& size) const
{
    if (size.width <= 1 && size.height <= 1) {
        return solid(pos.x, pos.y);
    }

    if (pos.x >= mWidth || pos.x + size.width - 1 >= mWidth || pos.y >= mHeight || pos.y + size.height - 1 >= mHeight) {
        return true;
    }

    for (uint32_t cx = 0; cx < size.width; ++cx) {
        // Left
        if (mSolid[pos.x + cx + (pos.y * mWidth)] || mSolid[pos.x + cx + ((pos.y + size.height - 1) * mWidth)]) {
            return true;
        }

        // Right
        if (mSolid[pos.x + size.width - cx - 1 + (pos.y * mWidth)] || mSolid[pos.x + size.width - cx - 1 + ((pos.y + size.height - 1) * mWidth)]) {
            return true;
        }
    }

    for (uint32_t cy = 0; cy < size.height; ++cy) {
        // Top
        if (mSolid[pos.x + ((pos.y + cy) * mWidth)] || mSolid[pos.x + size.width - 1 + ((pos.y + cy) * mWidth)]) {
            return true;
        }

        // Bottom
        if (mSolid[pos.x + size.height - cy - 1 + (pos.y * mWidth)] || mSolid[pos.x + (size.width - 1) + ((pos.y + size.height - cy - 1) * mWidth)]) {
            return true;
        }
    }

    return false;
}

bool CollisionMap::transparent(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const
{
    if (width == 1 && height == 1) {
        return bounds(x, y) && mTransparent[(y * mWidth) + x];
    }

    return false;
}

void CollisionMap::set(uint32_t x, uint32_t y, bool b)
{
    if (bounds(x, y)) {
        mSolid[(y * mWidth) + x] = b;
    }
}

#ifdef DEBUG
void CollisionMap::save(const std::string& path)
{
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> data;
    data.reserve(mWidth * mHeight * 4);

    for (uint32_t y = 0; y < mHeight; y++) {
        for (uint32_t x = 0; x < mWidth; x++) {
            uint8_t c = mSolid[x + (y * mWidth)] ? 0 : 255;
            data.push_back(c);
            data.push_back(c);
            data.push_back(c);
            data.push_back(255);
        }
    }

    lodepng::State state;
    state.encoder.filter_palette_zero = 0;
    state.encoder.add_id = 0;
    state.encoder.text_compression = 1;
    state.encoder.zlibsettings.nicematch = 258;
    state.encoder.zlibsettings.lazymatching = 1;
    state.encoder.zlibsettings.windowsize = 32768;

    std::vector<uint8_t> temp;
    state.encoder.filter_strategy = LFS_ZERO;
    state.encoder.zlibsettings.minmatch = 3;
    state.encoder.zlibsettings.btype = 2;
    state.encoder.auto_convert = 0;
    if (lodepng::encode(buffer, data, mWidth, mHeight, state) != 0) {
        return;
    }

    lodepng::save_file(buffer, path);
}
#endif
