#include <engine/types.h>

using namespace engine;

Types::Rect::Rect(int x, int y, int w, int h)
	: x(x)
	, y(y)
	, width(w)
	, height(h)
{

} 

Types::RectF::RectF(float x, float y, float w, float h)
	: x(x)
	, y(y)
	, width(w)
	, height(h)
{

}

Types::Point::Point(int x, int y)
	: x(x)
	, y(y)
{

}

Types::PointF::PointF(float x, float y)
	: x(x)
	, y(y)
{

}

Types::Dimension::Dimension(int w, int h)
	: width(w)
	, height(h)
{
	
}

Types::Cells::Cells(int cols, int rows)
	: cols(cols)
	, rows(rows)
{
	
}

Types::Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: r(r)
	, g(g)
	, b(b)
	, a(a)
{

}