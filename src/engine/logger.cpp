#include <mutex>

#include <engine/logger.h>

using namespace engine;

static std::mutex loggerMutex;

LoggerStream::LoggerStream(std::ostream& s, char type, std::string tag)
    : mOut(s)
    , mType(type)
    , mTag(std::move(tag))
{
}

LoggerStream::~LoggerStream()
{
    std::lock_guard<std::mutex> lock(loggerMutex);
    mOut << mType;
    if (!mTag.empty()) {
        mOut << "/" << mTag;
    }
    mOut << ": " << mStream.str() << std::endl
         << std::flush;
}

Logger::critical::critical(std::string tag)
    : LoggerStream(std::cerr, 'C', std::move(tag))
{
}

Logger::error::error(std::string tag)
    : LoggerStream(std::cerr, 'E', std::move(tag))
{
}

Logger::warning::warning(std::string tag)
    : LoggerStream(std::cerr, 'W', std::move(tag))
{
}

Logger::info::info(std::string tag)
    : LoggerStream(std::cout, 'I', std::move(tag))
{
}

#ifdef DEBUG
Logger::debug::debug(std::string tag)
    : LoggerStream(std::cout, 'D', std::move(tag))
{
}
#endif

Logger::script::script()
    : LoggerStream(std::cout, 'S', "")
{
}

void Logger::scriptPrint(const std::string& str)
{
    script() << str;
}