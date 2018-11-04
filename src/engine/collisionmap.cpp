#include <cmath>

#include <png++/png.hpp>

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
	mSolid.resize(mWidth * mHeight);
}

CollisionMap::CollisionMap(const std::string& path)
	: CollisionMap()
{
	png::image<png::gray_pixel> png(path);
	if (png.get_width() == 0)
	{
		return;
	}

	mWidth = png.get_width();
	mHeight = png.get_height();

	mSolid.resize(mWidth * mHeight);
	for (uint32_t y = 0; y < png.get_height(); y++)
	{
		for (uint32_t x = 0; x < png.get_width(); x++)
		{
			mSolid[(y * mWidth) + x] = (png[y][x] != 255);
		}
	}

	mValid = true;
}

bool CollisionMap::get(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Types::Quad* diff) const
{
	if (diff != nullptr)
	{
		diff->x1 = diff->y1 = diff->x2 = diff->y2 = -1;
	}

	if (width == 1 && height == 1)
	{
		return bounds(x, y) && mSolid[(y * mWidth) + x];
	}

	if (x >= mWidth || x + width - 1 >= mWidth || y >= mHeight || y + height - 1 >= mHeight)
	{
		diff->x1 = (x + width) - mWidth - 1;
		diff->y1 = (y + height) - mHeight - 1;
		diff->x2 = mWidth - (x + width) + 1;
		diff->y2 = mHeight - (y + height) + 1;
		return true;
	}

	bool found = false;
	for (uint32_t cx = 0; cx < width; cx++)
	{
		// Left
		if (diff == nullptr || diff->x1 == -1)
		{
			if (mSolid[x + (y * mWidth) + cx] || mSolid[x + ((y + height - 1) * mWidth) + cx])
			{
				if (diff == nullptr)
				{
					return true;
				}

				diff->x1 = cx;
				found = true;
			}
		}

		// Right
		if (diff == nullptr || diff->x2 == -1)
		{
			if (mSolid[x + (y * mWidth) + (width - cx - 1)] || mSolid[x + ((y + height - 1) * mWidth) + (width - cx - 1)])
			{
				if (diff == nullptr)
				{
					return true;
				}

				diff->x2 = cx;
				found = true;
			}
		}

		if (found && diff != nullptr && diff->x1 != -1 && diff->x2 != -1)
		{
			break;
		}
	}

	for (uint32_t cy = 0; cy < height; cy++)
	{
		// Top
		if (diff == nullptr || diff->y1 == -1)
		{
			if (mSolid[x + ((y + cy) * mWidth)] || mSolid[x + ((y + cy) * mWidth) + (width - 1)])
			{
				if (diff == nullptr)
				{
					return true;
				}

				diff->y1 = cy;
				found = true;
			}
		}

		// Bottom
		if (diff == nullptr || diff->y2 == -1)
		{
			if (mSolid[x + ((y + height - cy - 1) * mWidth)] || mSolid[x + ((y + height - cy - 1) * mWidth) + (width - 1)])
			{
				if (diff == nullptr)
				{
					return true;
				}

				diff->y2 = cy;
				found = true;
			}
		}

		if (found && diff != nullptr && diff->y1 != -1 && diff->y2 != -1)
		{
			break;
		}
	}

	if (found && diff != nullptr)
	{
		diff->x1 = (diff->x1 != -1) ? diff->x1 : 0;
		diff->y1 = (diff->y1 != -1) ? diff->y1 : 0;
		diff->x2 = (diff->x2 != -1) ? diff->x2 : 0;
		diff->y2 = (diff->y2 != -1) ? diff->y2 : 0;
	}

	return found;
}

void CollisionMap::set(uint32_t x, uint32_t y, bool b)
{
	if (bounds(x, y))
	{
		mSolid[(y * mWidth) + x] = b;
	}
}

void CollisionMap::save(const std::string& path)
{
	png::image<png::gray_pixel> out(mWidth, mHeight);
	for (uint32_t y = 0; y < mHeight; y++)
	{
		for (uint32_t x = 0; x < mWidth; x++)
		{
			out[y][x] = png::gray_pixel(get(x, y) ? 0 : 255);
		}
	}

	out.write(path);
}
