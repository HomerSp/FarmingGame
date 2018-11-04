cmake_minimum_required(VERSION 3.1.0)

find_path(Pngpp_INCLUDE_DIRS_PNG png.h
    HINTS
    PATH_SUFFIXES include
)

find_path(Pngpp_INCLUDE_DIRS_PP png.hpp
    HINTS
    PATH_SUFFIXES include/png++
)

if (Pngpp_INCLUDE_DIRS_PNG AND Pngpp_INCLUDE_DIRS_PP)
    set(Pngpp_INCLUDE_DIRS ${Pngpp_INCLUDE_DIRS_PNG} ${Pngpp_INCLUDE_DIRS_PP})
endif (Pngpp_INCLUDE_DIRS_PNG AND Pngpp_INCLUDE_DIRS_PP)

find_library(Pngpp_LIBRARY_PNG
    NAMES png
)

find_library(Pngpp_LIBRARY_16
    NAMES png16
)

if (Pngpp_LIBRARY_PNG AND Pngpp_LIBRARY_16)
    set(Pngpp_LIBRARY ${Pngpp_LIBRARY_16})
endif (Pngpp_LIBRARY_PNG AND Pngpp_LIBRARY_16)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Pngpp REQUIRED_VARS Pngpp_LIBRARY Pngpp_INCLUDE_DIRS)