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

eCritical::eCritical()
	: LoggerStream(std::cerr, "Critical")
{

}

eWarning::eWarning()
	: LoggerStream(std::cerr, "Warning")
{

}

eInfo::eInfo()
	: LoggerStream(std::cout, "Info")
{

}

eDebug::eDebug()
	: LoggerStream(std::cout, "Debug")
{

}
