#pragma once

#include <memory>

#include <png++/png.hpp>

#include <engine/image.h>
#include <engine/renderer.h>
#include <engine/map.h>
#include <engine/charset.h>

namespace engine
{
	class Character
	{
	public:
		struct Direction
		{
			typedef enum
			{
				Down = 0,
				Left,
				Right,
				Up,
			} Type;
		};

		Character(const std::string& name);

		void animate(uint64_t currentFrame);
		void draw(Renderer& renderer, const Types::Point& camera);

		void process(uint64_t frameDiff, Map* map = nullptr);
		void reset();
		void velocity(uint64_t frameDiff, int8_t x, int8_t y);

		float x() const
		{
			return mPos.x;
		}

		float y() const
		{
			return mPos.y;
		}

		int width() const
		{
			return mCharset->width(mCharsetType);
		}

		int height() const
		{
			return mCharset->height(mCharsetType);
		}

		bool isMoving() const;

		void turnTo(Direction::Type direction);

		void setDirection(Direction::Type direction);
		void setSpeed(float speed)
		{
			mSpeed = speed;
		}

		void setFriction(float friction)
		{
			mFriction = friction;
		}

		void setX(float x)
		{
			mPos.x = x;
		}

		void setY(float y)
		{
			mPos.y = y;
		}

		bool operator!() const
		{
			return !mValid;
		}

	protected:
		void updateVelocity(float& velocity, int8_t direction, float val);

	private:
		bool mValid;
		std::string mName;
		std::shared_ptr<Charset> mCharset;
		Charset::Type mCharsetType;
		int mFrame;
		std::shared_ptr<engine::Image> mPortrait;

		float mDirectionTurn;
		Direction::Type mDirectionTo;
		Direction::Type mDirection;

		float mSpeed;
		Types::PointF mPos;
		Types::PointF mVelocity;
		float mFriction;
	};
}
