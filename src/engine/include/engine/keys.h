#pragma once

#include <list>
#include <map>

namespace engine {
	struct Keys
	{
	public:
		typedef enum {
			None = 0,
			Down,
			Left,
			Right,
			Up,
			Run,
			Walk,
			Friction,
			Test,
		} Type;
	};

	class KeyList : public std::list<Keys::Type> {
	public:
		explicit KeyList();

		void append(Keys::Type key);
		void append(int key);

		void remove(int key);

		bool contains(Keys::Type key);

		Keys::Type &operator[](int type) {
			return mKeyTable[type];
		}

	private:
		std::map<int, Keys::Type> mKeyTable;
	};
}
