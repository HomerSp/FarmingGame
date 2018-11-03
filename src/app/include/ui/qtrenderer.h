#pragma once

#include <QPainter>

#include <engine/renderer.h>

class QtRenderer : public engine::Renderer
{
public:
	QtRenderer();

	int width();
	int height();

	void fillRect(const engine::Types::Rect& dst, const engine::Types::Color& color);

	void drawImage(const engine::Image& img, const engine::Types::Rect &src, const engine::Types::Rect &dst);

	void translate(float x, float y);

	void setPainter(QPainter* painter) {
		mPainter = painter;
	}

private:
	QPainter* mPainter;
};
