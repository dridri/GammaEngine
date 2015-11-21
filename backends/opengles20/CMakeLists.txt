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

add_library(backend_opengles20
	SHARED
	src/OpenGLES20Object.cpp
	src/OpenGLES20Renderer.cpp
	src/OpenGLES20Renderer2D.cpp
	src/OpenGLES20DeferredRenderer.cpp
	src/OpenGLES20Window.cpp
	src/OpenGLES20Instance.cpp
)

add_dependencies(backend_opengles20 gammaengine)

SET_TARGET_PROPERTIES(backend_opengles20 PROPERTIES PREFIX "")
target_link_libraries(backend_opengles20 gammaengine ${OPENGL_LIBRARIES} -pthread)

INSTALL(TARGETS backend_opengles20 ARCHIVE DESTINATION /usr/lib/gammaengine LIBRARY DESTINATION /usr/lib/gammaengine)
