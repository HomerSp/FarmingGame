cmake_minimum_required(VERSION 3.1.0)

find_path(JsonCpp_INCLUDE_DIRS json/json.h
	HINTS
	PATH_SUFFIXES include/jsoncpp
)

find_library(JsonCpp_LIBRARY
	NAMES jsoncpp
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(JsonCpp REQUIRED_VARS JsonCpp_LIBRARY JsonCpp_INCLUDE_DIRS)