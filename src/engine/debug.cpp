#include <engine/debug.h>

using namespace engine;

DebugStream::DebugStream(std::ostream &s, std::string type)
	: mOut(s)
	, mType(type)
{

}

DebugStream::~DebugStream() {
	mOut << mType << ": " << mStream.str() << std::endl << std::flush;
}

eCritical::eCritical()
	: DebugStream(std::cerr, "Critical")
{

}

eWarning::eWarning()
	: DebugStream(std::cerr, "Warning")
{

}

eInfo::eInfo()
	: DebugStream(std::cout, "Info")
{

}

eDebug::eDebug()
	: DebugStream(std::cout, "Debug")
{

}
