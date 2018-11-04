#include <engine/frametimer.h>

using namespace engine;

FrameTimer::FrameTimer()
	: mStart(std::chrono::high_resolution_clock::now())
{
	mSaved = mLast = elapsed();
}

uint64_t FrameTimer::diff() const
{
	return elapsed() - mLast;
}

uint64_t FrameTimer::elapsed() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - mStart).count();
}

uint64_t FrameTimer::last() const
{
	return mLast;
}

void FrameTimer::start()
{
	mSaved = elapsed();
}

void FrameTimer::end()
{
	mLast = mSaved;
}