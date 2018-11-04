#include <thread>

#include <engine/logger.h>
#include <engine/engine.h>

using namespace engine;

Engine::Engine(uint32_t width, uint32_t height)
	: mHasFocus(true)
	, mWidth(width)
	, mHeight(height)
	, mCameraX(0)
	, mCameraY(0)
	, mMap(nullptr)
	, mHero(nullptr)
{
	mMap = std::shared_ptr<engine::Map>(new engine::Map("map"));
	mHero = std::shared_ptr<engine::Character>(new engine::Character("hero"));
	mHero->setX((mMap->pixelWidth() - mHero->width()) / 2);
	mHero->setY((mMap->pixelHeight() - mHero->height()) / 2);
}

int Engine::bufferWidth() const
{
	return mMap->getTileDimension().width * 2;
}

int Engine::bufferHeight() const
{
	return mMap->getTileDimension().height * 2;
}

bool Engine::process()
{
	bool needRepaint = true;

	mFrameTimer.start();

	int8_t x = 0, y = 0;
	if(mHasFocus && !mDownKeys.empty()) {
		bool turned = false;
		for(auto it = mDownKeys.rbegin(); it != mDownKeys.rend(); it++) {
		   switch(*it) {
			case engine::Keys::Up:
				if(y == 0) {
					y = -1;
				}
				if(!turned) {
					mHero->turnTo(engine::Character::Direction::Up);
					turned = true;
				}
				break;
			case engine::Keys::Down:
				if(y == 0) {
					y = 1;
				}
				if(!turned) {
					mHero->turnTo(engine::Character::Direction::Down);
					turned = true;
				}
				break;
			case engine::Keys::Left:
				if(x == 0) {
					x = -1;
				}
				if(!turned) {
					mHero->turnTo(engine::Character::Direction::Left);
					turned = true;
				}
				break;
			case engine::Keys::Right:
				if(x == 0) {
					x = 1;
				}
				if(!turned) {
					mHero->turnTo(engine::Character::Direction::Right);
					turned = true;
				}
				break;
			default:
				break;
			}

			if(x != 0 && y != 0) {
				break;
			}
		}

		if(mDownKeys.contains(engine::Keys::Friction)) {
			mHero->setFriction(0.1f);
		} else {
			mHero->setFriction(1.0f);
		}

		if(mDownKeys.contains(engine::Keys::Run)) {
			mHero->setSpeed(2.0f);
		} else if(mDownKeys.contains(engine::Keys::Walk)) {
			mHero->setSpeed(0.5f);
		} else {
			mHero->setSpeed(1.0f);
		}
	} else {
		mHero->setSpeed(1.0f);
		mHero->reset();
	}

	mHero->velocity(mFrameTimer.diff(), x, y);
	mHero->process(mFrameTimer.diff(), mMap.get());

	if(!mHero->isMoving()) {
		mHero->reset();
	}

	if(mHero->isMoving()) {
		mHero->animate(mFrameTimer.elapsed());
	}

	mMap->animate(mFrameTimer.elapsed());

	int cameraX = mHero->x() - (mWidth / 2) + (mHero->width() / 2);
	if(cameraX < 0) {
		cameraX = 0;
	}

	mCameraX = cameraX;
	if(mCameraX >= mMap->pixelWidth() - mWidth)
	{
		mCameraX = mMap->pixelWidth() - mWidth;
	}

	int cameraY = mHero->y() - (mHeight / 2) + (mHero->height() / 2);
	if(cameraY < 0)
	{
		cameraY = 0;
	}

	mCameraY = cameraY;
	if(mCameraY >= mMap->pixelHeight() - mHeight)
	{
		mCameraY = mMap->pixelHeight() - mHeight;
	}

	if(mDownKeys.contains(engine::Keys::Test)) {
		std::this_thread::sleep_for (std::chrono::milliseconds(50));
	}

	mFrameTimer.end();

	return needRepaint;
}

void Engine::paint(Renderer& renderer)
{
	renderer.fillRect(Types::Rect(0, 0, mWidth, mHeight), Types::Color(0, 0, 0));

	// Centre small maps.
	float translateX = 0.0f, translateY = 0.0f;
	if(mMap->pixelWidth() < mWidth) {
		translateX = (mWidth / 2) - (mMap->pixelWidth() / 2);
	}

	if(mMap->pixelHeight() < mHeight) {
		translateY = (mHeight / 2) - (mMap->pixelHeight() / 2);
	}

	if(translateX != 0.0f || translateY != 0.0f) {
		renderer.translate(translateX, translateY);
	}

	mMap->draw(renderer, Types::Rect(mCameraX, mCameraY, mWidth, mHeight));
	mHero->draw(renderer, Types::Point(mCameraX, mCameraY));
}

void Engine::setKeyMap(const std::map<int, Keys::Type>& keys)
{
	for(auto it = keys.begin(); it != keys.end(); it++)
	{
		mDownKeys[it->first] = it->second;
	}
}

void Engine::setKeyDown(int key)
{
	mDownKeys.append(key);
}

void Engine::setKeyUp(int key)
{
	mDownKeys.remove(key);
}

void Engine::setSize(int width, int height)
{
	mWidth = width;
	mHeight = height;
}