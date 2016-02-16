cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L../../ -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-but-set-variable -Wno-unused-variable")

if("${release}" MATCHES "yes")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
        add_definitions(-DGE_RELEASE)
else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
endif()

include_directories(include)

add_library(backend_framebuffer
	SHARED
	src/FbObject.cpp
	src/FbRenderer.cpp
	src/FbRenderer2D.cpp
	src/FbDeferredRenderer.cpp
	src/FbWindow.cpp
	src/FbInstance.cpp
)

add_dependencies(backend_framebuffer gammaengine)

SET_TARGET_PROPERTIES(backend_framebuffer PROPERTIES PREFIX "")
target_link_libraries(backend_framebuffer gammaengine)

INSTALL(TARGETS backend_framebuffer ARCHIVE DESTINATION /usr/lib/gammaengine LIBRARY DESTINATION /usr/lib/gammaengine)
