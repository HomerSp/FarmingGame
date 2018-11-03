#pragma once

#include <memory>

#include <png++/png.hpp>

namespace engine {
	class Image {
	public:
		Image();
		Image(const std::string& path);
		~Image();

		uint8_t* data() const {
			return mData.get();
		}

		uint32_t dataSize() const {
			return mWidth * mHeight * 4;
		}

		int width() const {
			return mWidth;
		}

		int height() const {
			return mHeight;
		}

		bool operator!() const {
			return mWidth == 0;
		}

	private:
		int mWidth;
		int mHeight;
		std::shared_ptr<uint8_t> mData;
	};
}
