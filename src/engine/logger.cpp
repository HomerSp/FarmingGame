#include <engine/logger.h>

using namespace engine;

LoggerStream::LoggerStream(std::ostream& s, std::string type)
    : mOut(s)
    , mType(std::move(type))
{
}

LoggerStream::~LoggerStream()
{
    mOut << mType << ": " << mStream.str() << std::endl
         << std::flush;
}

Logger::critical::critical()
    : LoggerStream(std::cerr, "Critical")
{
}

Logger::error::error()
    : LoggerStream(std::cerr, "Error")
{
}

Logger::warning::warning()
    : LoggerStream(std::cerr, "Warning")
{
}

Logger::info::info()
    : LoggerStream(std::cout, "Info")
{
}

#ifdef DEBUG
Logger::debug::debug()
    : LoggerStream(std::cout, "Debug")
{
}
#endif
