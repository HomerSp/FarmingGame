#pragma once

#include <iostream>
#include <sstream>

namespace engine {
	class LoggerStream {
	public:
		~LoggerStream();

		template<class T>
	    LoggerStream &operator<<(const T &x) {
	    	if(mStream.tellp() > 0) {
	    		mStream << " ";
	    	}

		    mStream << x;
		    return *this;
		}

	protected:
		LoggerStream(std::ostream& s, std::string type);

	private:
		std::ostream &mOut;
		std::string mType;
		std::ostringstream mStream;
	};

	struct Logger
	{
	public:
		class critical : public LoggerStream {
		public:
			critical();
		};

		class warning : public LoggerStream {
		public:
			warning();
		};

		class info : public LoggerStream {
		public:
			info();
		};

		class debug : public LoggerStream {
		public:
			debug();
		};
	};
}
