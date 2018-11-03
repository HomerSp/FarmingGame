#pragma once

#include <cstdint>

namespace engine
{
	class Types
	{
	public:
		struct Rect {
		public:
			Rect(int x = 0, int y = 0, int w = 0, int h = 0);

			int x, y;
			int width, height;
		};

		struct RectF {
		public:
			RectF(float x = 0, float y = 0, float w = 0, float h = 0);

			float x, y;
			float width, height;
		};

		struct Point {
		public:
			Point(int x = 0, int y = 0);

			int x, y;
		};

		struct PointF {
		public:
			PointF(float x = 0, float y = 0);

			float x, y;
		};

		struct Dimension {
		public:
			Dimension(int w = 0, int h = 0);

			int width;
			int height;
		};

		struct Cells {
		public:
			Cells(int cols = 0, int rows = 0);

			int cols;
			int rows;
		};

		struct Color
		{
		public:
			Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

			uint8_t r, g, b, a;
		};
	};
} 
