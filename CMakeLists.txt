cmake_minimum_required(VERSION 2.8)

# User defined
project(OpenGLInterfaces)

include(myy/CMakeCommonOptions.txt)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set (MyyMakeTests_VERSION_MAJOR 2)
set (MyyMakeTests_VERSION_MINOR 0)

set(MyyProjectSources
    myy.c
    )

set(MyyProjectHeaders
   )

set(MyyFoldersToCopy
    shaders
    textures
    data)

if (MYY_ANDROID)
	find_library(Freetype REQUIRED)
else (MYY_ANDROID)
	find_package(Freetype REQUIRED)
endif (MYY_ANDROID)
include_directories(${FREETYPE_INCLUDE_DIRS})
set(MyyProjectLibDeps
    ${FREETYPE_LIBRARIES})

set(MyyShaders)
set(MyyData
    data/codepoints.dat)
set(MyyTextures
    textures/fonts.raw)
set(MyyDocumentation
	  README.md)
add_custom_target(Shaders  SOURCES ${MyyShaders})
add_custom_target(Textures SOURCES ${MyyTextures})
add_custom_target(Data     SOURCES ${MyyData})
add_custom_target(Headers  SOURCES ${MyyProjectHeaders})
add_custom_target(Documentation SOURCES ${MyyDocumentation})

include_directories(. myy)

# Common Section
include(myy/CMakePart.txt)
