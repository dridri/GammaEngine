cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L../../../libvulkangl/build -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-but-set-variable -Wno-unused-variable -I./fake")

if("${release}" MATCHES "yes")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
        add_definitions(-DGE_RELEASE)
else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
endif()

if(WIN32)
	add_definitions(-DGE_WIN32)
	include_directories(../../include/windows)
else()
	add_definitions(-DGE_LINUX)
	include_directories(../../include/linux)
endif()

include_directories(include)
include_directories(../../libvulkangl/include)

add_library(backend_vulkan
	SHARED
	src/VulkanObject.cpp
	src/VulkanRenderer.cpp
	src/VulkanWindow.cpp
	src/VulkanInstance.cpp
)

add_dependencies(backend_vulkan gammaengine)

SET_TARGET_PROPERTIES(backend_vulkan PROPERTIES PREFIX "")
target_link_libraries(backend_vulkan gammaengine -lvulkangl -pthread)

INSTALL(TARGETS backend_vulkan ARCHIVE DESTINATION /usr/lib/gammaengine LIBRARY DESTINATION /usr/lib/gammaengine)
