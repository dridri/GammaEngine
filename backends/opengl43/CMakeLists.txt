cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L../../ -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-but-set-variable -Wno-unused-variable -I./fake")

if("${release}" MATCHES "yes")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
        add_definitions(-DGE_RELEASE)
else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
endif()

find_package(OpenGL REQUIRED)

if(WIN32)
	add_definitions(-DGE_WIN32)
	include_directories(../../include/windows)
else()
	add_definitions(-DGE_LINUX)
	include_directories(../../include/linux)
endif()

include_directories(include)

add_library(backend_opengl43
	SHARED
	src/OpenGL43Object.cpp
	src/OpenGL43Renderer.cpp
	src/OpenGL43Renderer2D.cpp
	src/OpenGL43DeferredRenderer.cpp
	src/OpenGL43Window.cpp
	src/OpenGL43Instance.cpp
)

add_dependencies(backend_opengl43 gammaengine)

SET_TARGET_PROPERTIES(backend_opengl43 PROPERTIES PREFIX "")
target_link_libraries(backend_opengl43 gammaengine ${OPENGL_LIBRARIES} -pthread)

INSTALL(TARGETS backend_opengl43 ARCHIVE DESTINATION /usr/lib/gammaengine LIBRARY DESTINATION /usr/lib/gammaengine)
