#pragma once

#include <memory>
#include <vector>

#include <png++/png.hpp>

namespace engine
{
	class Image
	{
	public:
		Image();
		Image(const std::string& path);

		const unsigned char* data() const
		{
			return mData.data();
		}

		uint32_t dataSize() const
		{
			return mWidth * mHeight * 4;
		}

		uint32_t width() const
		{
			return mWidth;
		}

		uint32_t height() const
		{
			return mHeight;
		}

		bool operator!() const
		{
			return mWidth == 0;
		}

	private:
		uint32_t mWidth;
		uint32_t mHeight;
		std::vector<unsigned char> mData;
	};
}
