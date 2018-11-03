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

	class eCritical : public LoggerStream {
	public:
		eCritical();
	};

	class eWarning : public LoggerStream {
	public:
		eWarning();
	};

	class eInfo : public LoggerStream {
	public:
		eInfo();
	};

	class eDebug : public LoggerStream {
	public:
		eDebug();
	};
}
