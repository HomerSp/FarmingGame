#include <engine/logger.h>

using namespace engine;

LoggerStream::LoggerStream(std::ostream &s, std::string type)
	: mOut(s)
	, mType(type)
{

}

LoggerStream::~LoggerStream() {
	mOut << mType << ": " << mStream.str() << std::endl << std::flush;
}

Logger::critical::critical()
	: LoggerStream(std::cerr, "Critical")
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

Logger::debug::debug()
	: LoggerStream(std::cout, "Debug")
{

}
