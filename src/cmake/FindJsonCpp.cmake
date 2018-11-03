cmake_minimum_required(VERSION 3.1.0)

FIND_PATH(JsonCpp_INCLUDE_DIRS json/json.h
	HINTS
	PATH_SUFFIXES include/jsoncpp
)

FIND_LIBRARY(JsonCpp_LIBRARY
	NAMES jsoncpp
)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(JsonCpp REQUIRED_VARS JsonCpp_LIBRARY JsonCpp_INCLUDE_DIRS)