#pragma once

#include <iostream>
#include <sstream>

namespace engine {
class LoggerStream {
public:
    ~LoggerStream();

    template <class T>
    LoggerStream& operator<<(const T& x)
    {
        if (mStream.tellp() > 0) {
            mStream << " ";
        }

        mStream << x;
        return *this;
    }

protected:
    LoggerStream(std::ostream& s, char type, std::string tag);

private:
    std::ostream& mOut;
    char mType;
    std::string mTag;
    std::ostringstream mStream;
};

struct Logger {
public:
    class critical : public LoggerStream {
    public:
        critical(std::string tag);
    };

    class error : public LoggerStream {
    public:
        error(std::string tag);
    };

    class warning : public LoggerStream {
    public:
        warning(std::string tag);
    };

    class info : public LoggerStream {
    public:
        info(std::string tag);
    };

#ifdef DEBUG
    class debug : public LoggerStream {
    public:
        debug(std::string tag);
    };
#else
    class debug {
    public:
        debug(std::string tag) {}

        template <class T>
        debug& operator<<(const T& x)
        {
            return *this;
        }
    };
#endif

    class script : public LoggerStream {
    public:
        script();
    };

    static void scriptPrint(const std::string& str);
};
}
