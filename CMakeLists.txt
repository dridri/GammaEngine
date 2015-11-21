cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
project(gammaengine)

option(target "target")
option(backend "backend")
option(audio "audio")
option(release "release")
option(bullet "bullet")
option(static "static")

if ( "${audio}" MATCHES OFF )
	set( audio "yes" )
endif()

if ( "${bullet}" MATCHES OFF )
	set( bullet "yes" )
endif()

if ( "${target}" MATCHES "ios" )
	set( bullet "no" )
else()
	find_package(OpenGL REQUIRED)
endif()

find_package(PNG REQUIRED) #PNG16
find_package(JPEG REQUIRED)
find_package(Freetype REQUIRED)
find_package(LibMad REQUIRED)
if ( "${bullet}" MATCHES "yes" )
	find_package(Bullet REQUIRED)
endif()

include_directories(${FREETYPE_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS}/freetype)
include_directories(${PNG16_INCLUDE_DIR})
include_directories(${BULLET_INCLUDE_DIR})

set(mode SHARED)

if ( "${static}" MATCHES "yes" )
	set(mode STATIC)
endif()

if(${target} MATCHES OFF)
	if(WIN32)
		set(target windows)
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set(target mac)
	elseif(UNIX)
		set(target linux)
	endif()
endif()

add_definitions(-DGE_LIB)
if( NOT ${backend} MATCHES OFF )
	add_definitions(-DGE_STATIC_BACKEND=${backend})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-variable")
if(NOT "${target}" MATCHES "ios")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable")
endif()
if("${target}" MATCHES "windows")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -static-libstdc++")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()
if("${release}" MATCHES "yes")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
	add_definitions(-DGE_RELEASE)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
endif()

include_directories(.)
include_directories(include)
#include_directories(libvulkangl/include)

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )


set(INCLUDES
	include/Camera.h
	include/Debug.h
	include/DeferredRenderer.h
	include/File.h
	include/Font.h
	include/gememory.h
	include/Image.h
	include/ImageLoaderJpg.h
	include/ImageLoaderPng.h
	include/Input.h
	include/Instance.h
	include/Light.h
	include/Matrix.h
	include/MeshBuilder.h
	include/Object.h
	include/ObjectLoaderObj.h
	include/PhysicalBody.h
	include/PhysicalCollider.h
	include/PhysicalGraph.h
	include/Quaternion.h
	include/Renderer2D.h
	include/Renderer.h
	include/Scene.h
	include/SkyRenderer.h
	include/Socket.h
	include/Thread.h
# 	include/LayeredMusic.h
	include/MetaObject.h
	include/Timer.h
	include/Time.h
	include/Vector.h
	include/Vertex.h
	include/Window.h
)

set(INCLUDES_TARGET
	include/${target}/BaseWindow.h
	include/${target}/AudioOutput.h
)

set(SOURCES ${SOURCES}
# 	src/gememory.cpp
	src/Instance.cpp
	src/Light.cpp
	src/Scene.cpp
	src/Camera.cpp
	src/SkyRenderer.cpp
	src/MeshBuilder.cpp
	src/Object.cpp
	src/ObjectLoaderObj.cpp
	src/Image.cpp
	src/ImageLoaderPng.cpp
	src/ImageLoaderJpg.cpp
	src/Font.cpp
	src/FontLoaderTtf.cpp
	src/Vertex.cpp
	src/Vector.cpp
	src/Quaternion.cpp
	src/Matrix.cpp
	src/File.cpp
	src/Timer.cpp
	src/Time.cpp
	src/Input.cpp
	src/Socket.cpp
	src/Thread.cpp
	src/MetaObject.cpp
	src/Debug.cpp
# 	fake/dummy_vk.cpp
)

if ( "${audio}" MATCHES "yes" )
	set(INCLUDES ${INCLUDES}
		include/Music.h
		include/Sound.h
		include/AudioLoaderWav.h
		include/AudioLoaderMp3.h
	)
	set(SOURCES ${SOURCES}
	# 	src/LayeredMusic.cpp
		src/Music.cpp
		src/Sound.cpp
		src/AudioLoaderWav.cpp
		src/AudioLoaderMp3.cpp
	)
endif()

set(LIBS ${LIBS}
	${PNG16_LIBRARIES}
	${JPEG_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${LIBMAD_LIBRARY}
	-pthread
)

if ( "${bullet}" MATCHES "yes" )
	set(SOURCES ${SOURCES} src/PhysicalBody.cpp src/PhysicalGraph.cpp src/PhysicalCollider.cpp)
	set(LIBS ${LIBS} ${BULLET_LIBRARIES})
endif()

if("${target}" MATCHES "windows")
	add_definitions(-DGE_WIN32)
	include_directories(include/windows)
	set(LIBS ${LIBS} ${ZLIB_LIBRARY} -lwinmm -lwsock32 -static pthread)
	set(SOURCES ${SOURCES} src/windows/BaseWindow.cpp)
	if ( "${audio}" MATCHES "yes" )
		set(SOURCES ${SOURCES} src/windows/AudioOutput.cpp)
	endif()
elseif("${target}" MATCHES "ios")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++ -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1 -D__IPHONE_OS_VERSION_MIN_REQUIRED=60000")
#	set(CMAKE_CXX_FLAGS "-fobjc-abi-version=2 -fobjc-arc -nostdinc -I${CMAKE_IOS_SDK_ROOT}/usr/include -I${CMAKE_IOS_SDK_ROOT}/usr/include/c++/4.2.1 ${CMAKE_CXX_FLAGS} -Wno-unused-private-field -Wno-overloaded-virtual")
	set(CMAKE_CXX_FLAGS "-I./include -fobjc-abi-version=2 -fobjc-arc -nostdinc -I${CMAKE_IOS_SDK_ROOT}/usr/include ${CMAKE_CXX_FLAGS} -Wno-unused-private-field -Wno-overloaded-virtual -Wno-ignored-attributes -Wno-missing-braces")
	add_definitions(-DGE_IOS)
	set(LIBS ${LIBS} -pthread)
	include_directories(include/ios)
	set(SOURCES ${SOURCES} src/ios/AppDelegate.mm)
	set(SOURCES ${SOURCES} src/ios/BaseWindow.cpp)
	if ( "${audio}" MATCHES "yes" )
		set(SOURCES ${SOURCES} src/ios/AudioOutput.cpp)
	endif()
	set(mode STATIC)
else()
	find_package(ALSA REQUIRED)
	add_definitions(-DGE_LINUX)
	include_directories(include/linux)
	set(LIBS ${OPENGL_LIBRARIES} ${LIBS} ${ALSA_LIBRARY} -lX11 -lXext -lXrender -ldl -pthread)
	set(SOURCES ${SOURCES} src/linux/BaseWindow.cpp)
	if ( "${audio}" MATCHES "yes" )
		set(SOURCES ${SOURCES} src/linux/AudioOutput.cpp)
		set(LIBS ${LIBS} ${ALSA_LIBRARY})
	endif()
endif()

message(STATUS "LIBS : ${LIBS}")
set( LIB_NAME "gammaengine" )

if( ${backend} MATCHES OFF )
#	add_subdirectory(backends/vulkan)
	add_subdirectory(backends/opengl43)
	add_subdirectory(backends/opengles20)
else()
	file(GLOB BSOURCES backends/${backend}/src/*)
	set(SOURCES ${SOURCES} ${BSOURCES})
	include_directories(backends/${backend}/include)
	set(LIBS ${LIBS} ${OPENGL_LIBRARIES})
	set( LIB_NAME "gammaengine_${backend}" )
endif()

add_library(${LIB_NAME} "${mode}" ${SOURCES})
target_link_libraries(${LIB_NAME} ${LIBS})

if(NOT ("${target}" MATCHES "ios" OR "${target}" MATCHES "android"))
add_executable(testexec src/main.cpp)
target_link_libraries(testexec ${LIB_NAME} ${LIBS})
endif()

INSTALL(TARGETS ${LIB_NAME} ARCHIVE DESTINATION /usr/lib LIBRARY DESTINATION /usr/lib)
INSTALL(FILES ${INCLUDES} DESTINATION /usr/include/gammaengine)
# INSTALL(FILES ${INCLUDES_TARGET} DESTINATION /usr/include/gammaengine/${target})
INSTALL(FILES ${INCLUDES_TARGET} DESTINATION /usr/include/gammaengine)
