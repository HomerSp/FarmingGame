#include "include/utils.h"

bool Utils::createParentDir(const std::string& filePath, mode_t mode) 
{
	std::vector<std::string> dirs;
	splitString(filePath, "/", dirs);
	dirs.pop_back();

	std::string path = "/";

	struct stat st;
	for(auto it = dirs.begin(); it != dirs.end(); it++) {
		path += *it;

		if(stat(path.c_str(), &st) != 0) {
			mkdir(path.c_str(), mode);
		}

		path += "/";
	}

	return true;
} 

void Utils::splitString(const std::string &path, const std::string& delim, std::vector<std::string>& out) {
	uint32_t start = 0;
	auto end = path.find(delim);
	while(end != std::string::npos) {
		std::string v = path.substr(start, end - start);
		if(v.length() > 0) {
			out.push_back(v);
		}

		start += delim.length() + v.length();
		end = path.find(delim, start);
	}

	if(start + delim.length() < path.length()) {
		out.push_back(path.substr(start + delim.length()));
	}
}
