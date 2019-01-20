# Copyright (c) 2019 Miouyouyou
# 
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

# This is currently called through regen_assets.sh
# The "best" way would be to call this from the main CMakeLists.txt.
# However, since these tools are only used to generate assets,
# these should be built "in-place".

# I guess that I could run "cmake ." and "cmake --build ." from
# the tools directory using add_custom_command and add_custom_target
# But toying endlessly with CMake makes me rage after sometime.

cmake_minimum_required(VERSION 2.8)

# User defined
project(MyyAssetsTools)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 1)

find_package(Freetype REQUIRED)

include_directories(${FREETYPE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/..)
add_definitions(-DNDEBUG)

add_executable(font-packer
	font-packer.c
	../myy/helpers/vector.c
	../myy/helpers/strings.c
	../myy/platforms/linux/common/helpers/file.c)

add_executable(bmp-to-raw
	bmp-converter.c
	../myy/helpers/memory.c
	../myy/platforms/linux/common/helpers/file.c)

target_link_libraries(font-packer ${FREETYPE_LIBRARIES})
