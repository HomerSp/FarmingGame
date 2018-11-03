#pragma once

#include <chrono>
#include <string>

namespace engine {
	class FrameTimer {
	public:
		FrameTimer();

		uint64_t diff() const;
		uint64_t elapsed() const;
		uint64_t last() const;

		void start();
		void end();

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
		uint64_t mSaved;
		uint64_t mLast;
	};
}
