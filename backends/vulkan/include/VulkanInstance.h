/* WARNING
 * THIS IS A DRAFT, ALL THE REFERENCES USED HERE ARE PROBABLY ALL WRONG
 * All the code here is based on Mantle API samples, which Vulkan is more or less based on.
 * You should not use this file as a reference or example, and let it here
 * until Vulkan is officialy released
 * WARNING
 */

/*
 * The GammaEngine Library 2.0 is a multiplatform Vulkan-based game engine
 * Copyright (C) 2015  Adrien Aubry <dridri85@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef VULKANINSTANCE_H
#define VULKANINSTANCE_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "Instance.h"
#include "Thread.h"
#include <vulkan/vulkan.h>

namespace GE {
	template <typename T> class ProxyWindow;
	class BaseWindow;
	typedef ProxyWindow< BaseWindow > Window;
	class Renderer;
	class Vertex;
	class Object;
	class Image;
	class VulkanFramebuffer;
}
using namespace GE;
	void AffectVRAM( int64_t sz );

class VulkanInstance : public Instance
{
public:
	VulkanInstance( void* pBackend, const char* appName, uint32_t appVersion );
	virtual ~VulkanInstance(){}

	virtual int EnumerateGpus();
	virtual Instance* CreateDevice( int devid, int queueCount = 1 );
	virtual uint64_t ReferenceImage( Image* image );
	virtual void UnreferenceImage( uint64_t ref );
	virtual void UpdateImageData( Image* image, uint64_t ref );

	VkInstance instance() const { return mInstance; }
	VkPhysicalDevice gpu() const { return mGpus[mDeviceId]; }
	VkDevice device() const { return mDevices[mDeviceId]; }
	int32_t findQueueFamilyIndex( std::function< bool( uint32_t, VkQueueFamilyProperties* ) > cb );
	VkQueue graphicsQueue() const { return mGraphicsQueue; }
	VkQueue presentationQueue() const { return mPresentationQueue; }
	VkCommandPool commandPool() const { return mCommandPool; }
	uint32_t graphicsQueueFamilyIndex() const { return mGraphicsQueueFamilyIndex; }
	uint32_t presentationQueueFamilyIndex() const { return mPresentationQueueFamilyIndex; }
	VkRenderPass renderPass() const { return mRenderPass; }
	VkRenderPass clearRenderPass() const { return mClearRenderPass; }

	std::map< Thread*, VulkanFramebuffer* >& boundFramebuffers() { return mBoundFramebuffers; }
	VulkanFramebuffer* boundFramebuffer( Thread* thread ) { return mBoundFramebuffers[thread]; }

	uint32_t FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties );
	VkResult CreateBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory );
	VkResult CopyBuffer( VkBuffer dstBuffer, VkBuffer srcBuffer, VkDeviceSize offset, VkDeviceSize size );
	void TransitionImageLayout( VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout );
// 	VK_MEMORY_REF AllocateObject( VK_OBJECT object );
// 	VK_MEMORY_REF AllocateMappableBuffer( size_t size );
// 	static void UpdateDescriptorSet( VK_DESCRIPTOR_SET descriptorSet, VK_MEMORY_VIEW_ATTACH_INFO* memoryViewAttachInfo );

// 	void QueueSubmit( VK_CMD_BUFFER buf, VK_MEMORY_REF* refs, int nRefs );

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData );
	static void* sAlloc( size_t size, size_t align, int32_t allocType );
	static void sFree( void* pMem );

// 	VK_APPLICATION_INFO mAppInfo;
// 	VK_ALLOC_CALLBACKS mAllocCb;
// 	VK_INSTANCE mInstance;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	std::string mAppName;
	uint32_t mAppVersion;
	VkInstance mInstance;
	std::vector< VkPhysicalDevice > mGpus;
	std::vector< VkQueueFamilyProperties > mQueueFamilyProperties;
	std::vector< VkDevice > mDevices;
	int32_t mDeviceId;
	int32_t mGraphicsQueueFamilyIndex;
	int32_t mPresentationQueueFamilyIndex;
	VkQueue mGraphicsQueue;
	VkQueue mPresentationQueue;
	VkCommandPool mCommandPool;
	VkCommandBuffer mCommandBuffer;

	VkRenderPass mRenderPass;
	VkRenderPass mClearRenderPass;
	std::map< Thread*, VulkanFramebuffer* > mBoundFramebuffers;
};

#endif // VULKANINSTANCE_H
