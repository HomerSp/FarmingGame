#include <mutex>

#include <engine/logger.h>

using namespace engine;

static std::mutex loggerMutex;

LoggerStream::LoggerStream(std::ostream& s, std::string type, std::string tag)
    : mOut(s)
    , mType(std::move(type))
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
    : LoggerStream(std::cerr, "Critical", std::move(tag))
{
}

Logger::error::error(std::string tag)
    : LoggerStream(std::cerr, "Error", std::move(tag))
{
}

Logger::warning::warning(std::string tag)
    : LoggerStream(std::cerr, "Warning", std::move(tag))
{
}

Logger::info::info(std::string tag)
    : LoggerStream(std::cout, "Info", std::move(tag))
{
}

#ifdef DEBUG
Logger::debug::debug(std::string tag)
    : LoggerStream(std::cout, "Debug", std::move(tag))
{
}
#endif

Logger::script::script()
    : LoggerStream(std::cout, "Script", "")
{
}

void Logger::scriptPrint(const std::string& str)
{
    script() << str;
}