#pragma once

#include <sys/stat.h>
#include <string>
#include <vector>

class Utils
{
public:
	static bool createParentDir(const std::string& filePath, mode_t mode = 0755);

private:
	static void splitString(const std::string &path, const std::string& delim, std::vector<std::string>& out);
};
