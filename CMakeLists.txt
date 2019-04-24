cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
project(gammaengine)

option(target "target")
option(backend "backend")
option(audio "audio")
option(release "release")
option(bullet "bullet")
option(static "static")
option(notest "notest")

option(png "png")
option(jpeg "jpeg")
option(tga "tga")
option(freetype "freetype")
option(mad "mad")

if ( "${audio}" MATCHES OFF )
	set( audio "yes" )
	add_definitions( -DGE_AUDIO )
endif()

if ( "${bullet}" MATCHES OFF )
	set( bullet "yes" )
	add_definitions( -DGE_BULLET )
endif()

if ( "${png}" MATCHES OFF )
	set( png "yes" )
	add_definitions( -DGE_PNG )
endif()
if ( "${jpeg}" MATCHES OFF )
	set( jpeg "yes" )
	add_definitions( -DGE_JPEG )
endif()
if ( "${tga}" MATCHES OFF )
	set( tga "yes" )
	add_definitions( -DGE_TGA )
endif()
if ( "${freetype}" MATCHES OFF )
	set( freetype "yes" )
	add_definitions( -DGE_FREETYPE )
endif()
if ( "${mad}" MATCHES OFF )
	set( mad "yes" )
	add_definitions( -DGE_MAD )
endif()

if ( "${target}" MATCHES "ios" )
	set( bullet "no" )
elseif ( "${target}" MATCHES "rpi*" )
	include_directories( /opt/vc/include )
	include_directories( /opt/vc/include/interface/vcos )
	include_directories( /opt/vc/include/interface/vcos/pthreads )
else()
	find_package(OpenGL REQUIRED)
endif()

if ( "${png}" MATCHES "yes" )
	find_package(PNG REQUIRED) #PNG16
	include_directories(${PNG_INCLUDE_DIR})
endif()
if ( "${jpeg}" MATCHES "yes" )
	find_package(JPEG REQUIRED)
endif()
if ( "${freetype}" MATCHES "yes" )
	find_package(Freetype REQUIRED)
	include_directories(${FREETYPE_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS}/freetype)
endif()
if ( "${mad}" MATCHES "yes" )
	find_package(LibMad REQUIRED)
endif()
if ( "${bullet}" MATCHES "yes" )
	find_package(Bullet REQUIRED)
	include_directories(${BULLET_INCLUDE_DIR})
endif()

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

set_property( GLOBAL PROPERTY target "${target}" )

add_definitions(-DGE_LIB)
if( NOT ${backend} MATCHES OFF )
	add_definitions(-DGE_STATIC_BACKEND=${backend})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-pmf-conversions")
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

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath=./ -Wl,-rpath=/usr/lib/gammaengine -Wl,-rpath=/usr/local/lib/gammaengine" )
set( CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -Wl,-rpath=./ -Wl,-rpath=/usr/lib/gammaengine -Wl,-rpath=/usr/local/lib/gammaengine" )
set( CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -Wl,-rpath=./ -Wl,-rpath=/usr/lib/gammaengine -Wl,-rpath=/usr/local/lib/gammaengine" )

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
	include/FramebufferWindow.h
)

set(INCLUDES_TARGET
	include/${target}/BaseWindow.h
	include/${target}/AudioOutput.h
)

set(SOURCES ${SOURCES}
# 	src/gememory.cpp
	src/Instance.cpp
	src/RenderBuffer.cpp
	src/Light.cpp
	src/Scene.cpp
	src/Camera.cpp
	src/SkyRenderer.cpp
	src/MeshBuilder.cpp
	src/Object.cpp
	src/ObjectLoaderObj.cpp
	src/Image.cpp
	src/Font.cpp
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
	src/FramebufferWindow.cpp
# 	fake/dummy_vk.cpp
)

if ( "${audio}" MATCHES "yes" )
	set(INCLUDES ${INCLUDES}
		include/Music.h
		include/Sound.h
		include/AudioLoaderWav.h
	)
	set(SOURCES ${SOURCES}
	# 	src/LayeredMusic.cpp
		src/Music.cpp
		src/Sound.cpp
		src/AudioLoaderWav.cpp
	)
endif()

set( LIBS ${LIBS} -pthread )

if ( "${png}" MATCHES "yes" )
	set( INCLUDES ${INCLUDES} include/ImageLoaderPng.h )
	set( SOURCES ${SOURCES} src/ImageLoaderPng.cpp )
	set( LIBS ${LIBS} ${PNG_LIBRARIES} )
endif()
if ( "${jpeg}" MATCHES "yes" )
	set( INCLUDES ${INCLUDES} include/ImageLoaderJpg.h )
	set( SOURCES ${SOURCES} src/ImageLoaderJpg.cpp )
	set( LIBS ${LIBS} ${JPEG_LIBRARIES} )
endif()
if ( "${tga}" MATCHES "yes" )
	set( INCLUDES ${INCLUDES} include/ImageLoaderTga.h )
	set( SOURCES ${SOURCES} src/ImageLoaderTga.cpp )
endif()
if ( "${freetype}" MATCHES "yes" )
	set( SOURCES ${SOURCES} src/FontLoaderTtf.cpp )
	set( LIBS ${LIBS} ${FREETYPE_LIBRARIES} )
endif()
if ( "${audio}" MATCHES "yes" AND "${mad}" MATCHES "yes" )
	set( INCLUDES ${INCLUDES} include/AudioLoaderMp3.h )
	set( SOURCES ${SOURCES} src/AudioLoaderMp3.cpp )
	set( LIBS ${LIBS} ${LIBMAD_LIBRARY} )
endif()

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
	if ( "${target}" MATCHES "._egl")
		add_definitions(-DGE_LINUX)
		add_definitions(-DGE_EGL)
		set(SOURCES ${SOURCES} src/egl/BaseWindow.cpp)
	else()
		add_definitions(-DGE_LINUX)
		set(SOURCES ${SOURCES} src/linux/BaseWindow.cpp)
		set(LIBS ${OPENGL_LIBRARIES} ${LIBS} ${ALSA_LIBRARY} -lX11 -lXext -lXrender -pthread)
		if( ${backend} MATCHES OFF )
			set(LIBS ${LIBS} -ldl )
		endif()
	endif()
	include_directories(include/linux)
	if ( "${audio}" MATCHES "yes" )
		find_package(ALSA REQUIRED)
		set(SOURCES ${SOURCES} src/linux/AudioOutput.cpp)
		set(LIBS ${LIBS} ${ALSA_LIBRARY})
	endif()
endif()

message(STATUS "LIBS : ${LIBS}")
set( LIB_NAME "gammaengine" )

if( ${backend} MATCHES OFF )
#	add_subdirectory(backends/vulkan)
	if ( NOT ${target} MATCHES "rpi*" AND NOT ${target} MATCHES "android" AND NOT ${target} MATCHES "ios" )
		add_subdirectory(backends/opengl43)
		add_subdirectory(backends/vulkan)
	endif()
	add_subdirectory(backends/opengles20)
	add_subdirectory(backends/framebuffer)
else()
	file(GLOB BSOURCES backends/${backend}/src/*)
	set(SOURCES ${SOURCES} ${BSOURCES})
	include_directories(backends/${backend}/include)
	set(LIBS ${LIBS} ${OPENGL_LIBRARIES})
	set( LIB_NAME "gammaengine_${backend}" )
endif()

add_library(${LIB_NAME} "${mode}" ${SOURCES})
target_link_libraries(${LIB_NAME} ${LIBS})

if( NOT ("${target}" MATCHES "ios" OR "${target}" MATCHES "android") )
	if ( NOT "${notest}" MATCHES "yes" AND NOT "${notest}" MATCHES "1" )
		add_executable(testexec src/main.cpp)
		target_link_libraries(testexec ${LIB_NAME} ${LIBS})
		add_executable(osvr src/main_osvr.cpp)
		target_link_libraries(osvr osvrJointClientKit osvrClientKit ${LIB_NAME} ${LIBS})
	endif()
endif()

INSTALL(TARGETS ${LIB_NAME} ARCHIVE DESTINATION /usr/lib LIBRARY DESTINATION /usr/lib)
INSTALL(FILES ${INCLUDES} DESTINATION /usr/include/gammaengine)
# INSTALL(FILES ${INCLUDES_TARGET} DESTINATION /usr/include/gammaengine/${target})
INSTALL(FILES ${INCLUDES_TARGET} DESTINATION /usr/include/gammaengine)
