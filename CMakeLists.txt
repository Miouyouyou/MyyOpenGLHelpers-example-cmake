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

set(MyyFoldersToCopy)

if (MYY_ANDROID)
	find_library(Freetype REQUIRED)
else (MYY_ANDROID)
	find_package(Freetype REQUIRED)
endif (MYY_ANDROID)
include_directories(${FREETYPE_INCLUDE_DIRS})
set(MyyProjectLibDeps
    ${FREETYPE_LIBRARIES})

set(MyyShaders
	shaders/heatmap.frag
	shaders/heatmap.vert
	shaders/standard.frag
	shaders/standard.vert
	shaders/text.frag
	shaders/text.vert)
set(MyyData
	data/shaders.pack
	data/font_pack_meta.dat)
set(MyyTextures
	textures/fonts_bitmap.myyraw
	textures/TestTextureAtlas.myyraw)
set(MyyDocumentation
	  README.md)
add_custom_target(Shaders  SOURCES ${MyyShaders})
add_custom_target(Textures SOURCES ${MyyTextures})
add_custom_target(Data     SOURCES ${MyyData})
add_custom_target(Headers  SOURCES ${MyyProjectHeaders})
add_custom_target(Documentation SOURCES ${MyyDocumentation})

include_directories(${CMAKE_SOURCE_DIR}/. ${CMAKE_SOURCE_DIR}/myy)

add_custom_target(Assets ALL
	BYPRODUCTS ${MyyData} ${MyyTextures}
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/regen_assets.sh
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${MyyShaders} ${MyyData}
	SOURCES ${MyyShaders})
add_custom_command(TARGET Assets PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${MyyData} ${CMAKE_CURRENT_BINARY_DIR}/data/
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(TARGET Assets PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${MyyTextures} ${CMAKE_CURRENT_BINARY_DIR}/textures/
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Common Section
include(myy/CMakePart.txt)
