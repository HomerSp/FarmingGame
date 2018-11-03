#pragma once

#include <iostream>
#include <sstream>

namespace engine {
	class DebugStream {
	public:
		~DebugStream();

		template<class T>
	    DebugStream &operator<<(const T &x) {
	    	if(mStream.tellp() > 0) {
	    		mStream << " ";
	    	}

		    mStream << x;
		    return *this;
		}

	protected:
		DebugStream(std::ostream& s, std::string type);

	private:
		std::ostream &mOut;
		std::string mType;
		std::ostringstream mStream;
	};

	class eCritical : public DebugStream {
	public:
		eCritical();
	};

	class eWarning : public DebugStream {
	public:
		eWarning();
	};

	class eInfo : public DebugStream {
	public:
		eInfo();
	};

	class eDebug : public DebugStream {
	public:
		eDebug();
	};
}
