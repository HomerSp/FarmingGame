#include <cmath>
#include <fstream>

#include <json/json.h>

#include <engine/debug.h>
#include <engine/assetmanager.h>
#include <engine/character.h>

using namespace engine;

Character::Character(const std::string& name)
	: mValid(false)
	, mName("")
	, mCharsetType(Charset::TypeWalk)
	, mFrame(1)
	, mDirectionTurn(0.0f)
	, mDirectionTo(Direction::Down)
	, mDirection(Direction::Down)
	, mSpeed(1.0f)
	, mPos({0, 0})
	, mVelocity({0, 0})
	, mFriction(1.0f)
{
	std::shared_ptr<Json::Value> docPtr = AssetManager::json(AssetManager::Character, name);
	Json::Value doc = *docPtr;
	if(!doc.isObject() || !doc.isMember("name") || !doc.isMember("charset")) {
		eCritical() << "Invalid JSON data for character" << name;
		return;
	}

	mName = doc["name"].asString();
	mCharset = std::shared_ptr<Charset>(new Charset(doc["charset"].asString()));

	mValid = true;
}

Character::~Character() {

}

void Character::animate(uint64_t currentFrame) {
	int cols = mCharset->columns(mCharsetType);
	int frame = std::floor(currentFrame % ((cols + cols - 2) * 200) / 200);
	if(frame >= cols) {
		mFrame = frame + 1 - cols;
	} else {
		mFrame = frame;
	}
}

void Character::draw(Renderer& renderer, const Types::Point& camera) {
	Types::Point pos(mPos.x - camera.x, mPos.y - camera.y);
	mCharset->draw(renderer, pos, mCharsetType, mDirection, mFrame);
}

void Character::process(uint64_t frameDiff, Map* map) {
	if(mDirectionTo != mDirection && (mVelocity.x != 0.0f || mVelocity.y != 0.0f)) {
		mDirectionTurn += 1.0f * (frameDiff / 100.0f);
		if(mDirectionTurn >= 1.0f) {
			mDirectionTurn = 0.0f;

			if(mDirection == Direction::Up && mDirectionTo == Direction::Down) {
				mDirection = Direction::Left;
			} else if(mDirection == Direction::Down && mDirectionTo == Direction::Up) {
				mDirection = Direction::Right;
			} else if(mDirection == Direction::Left && mDirectionTo == Direction::Right) {
				mDirection = Direction::Up;
			} else if(mDirection == Direction::Right && mDirectionTo == Direction::Left) {
				mDirection = Direction::Down;
			} else {
				mDirection = mDirectionTo;
			}
		}
	}

    if(mVelocity.x != 0.0f || mVelocity.y != 0.0f) {
    	Types::PointF dst(mVelocity.x * (frameDiff / 5.0f), mVelocity.y * (frameDiff / 5.0f));
    	if(map != nullptr)
    	{
    		Types::PointF pos(mPos.x, mPos.y);
    		Types::Point size(width(), height());
    		map->checkCollision(pos, size, dst, mVelocity);
    	}

    	mPos.x += dst.x;
    	mPos.y += dst.y;
    }
}

void Character::reset() {
	mFrame = 1;
}

void Character::velocity(uint64_t frameDiff, int8_t x, int8_t y) {
	float val = 1.0f * (frameDiff / 200.0f);

	updateVelocity(mVelocity.x, x, val);
	updateVelocity(mVelocity.y, y, val);
}

bool Character::isMoving() const {
	return mVelocity.x != 0.0f || mVelocity.y != 0.0f;
}

void Character::turnTo(Direction::Type direction) {
	if(mDirection != direction && mDirectionTo != direction) {
		mDirectionTo = direction;
		mDirectionTurn = 0.0f;
	}
}

void Character::setDirection(Direction::Type direction) {
	mDirection = mDirectionTo = direction;
	mDirectionTurn = 0.0f;
}

void Character::updateVelocity(float& velocity, int8_t direction, float val)
{
	// Check if we are changing direction, this will increase the friction.
	if((direction > 0 && velocity < 0.0f) || (direction < 0 && velocity > 0.0f)) {
		val *= mFriction * 5;
	}

	if(direction > 0) {
		if(velocity + val < mSpeed) {
			velocity += val;
		} else if(velocity - val > mSpeed) {
			velocity -= val;
		} else {
			velocity = mSpeed;
		}
	} else if(direction < 0) {
		if(velocity - val > -mSpeed) {
			velocity -= val;
		} else if(velocity + val < -mSpeed) {
			velocity += val;
		} else {
			velocity = -mSpeed;
		}

	// No longer moving, increase the friction.
	} else if(velocity != 0.0f) {
		val *= mFriction * 5;
		if(velocity > 0.0f) {
			if(velocity - val > 0.0f) {
				velocity -= val;
			} else {
				velocity = 0.0f;
			}
		} else {
			if(velocity + val < 0.0f) {
				velocity += val;
			} else {
				velocity = 0.0f;
			}
		}
	}
}