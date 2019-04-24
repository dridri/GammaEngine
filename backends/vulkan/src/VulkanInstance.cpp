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

#include <execinfo.h>
#include "VulkanInstance.h"
#include "Vertex.h"
#include "Window.h"
#include "VulkanWindow.h"
#include "Debug.h"
#include "Image.h"
#include "VulkanRenderPass.h"

#ifndef ALIGN
	#define ALIGN(x, align) (((x)+((align)-1))&~((align)-1))
#endif

extern "C" GE::Instance* CreateInstance( void* pBackend, const char* appName, uint32_t appVersion ) {
	return new VulkanInstance( pBackend, appName, appVersion );
}


VulkanInstance::VulkanInstance( void* pBackend, const char* appName, uint32_t appVersion )
	: Instance( pBackend )
	, mAppName( appName )
	, mAppVersion( appVersion )
	, mDeviceId( -1 )
{
	fDebug( appName, appVersion );

	if ( !mBaseInstance || mBaseInstance->device() == 0 ) {
		mBaseInstance = this;
	}


	std::vector< const char* > extensions;
	extensions.push_back( "VK_KHR_surface" );
	extensions.push_back( "VK_KHR_xlib_surface" );
	extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
	extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

	// initialize the VkApplicationInfo structure
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = appName;
	app_info.applicationVersion = appVersion;
	app_info.pEngineName = "GammaEngine";
	app_info.engineVersion = 1;
	app_info.apiVersion = VK_API_VERSION_1_0;

	// initialize the VkInstanceCreateInfo structure
	VkInstanceCreateInfo inst_info = {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = nullptr;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = extensions.size();
	inst_info.ppEnabledExtensionNames = extensions.data();
	const char* layers[] = { "VK_LAYER_LUNARG_standard_validation" };
	inst_info.ppEnabledLayerNames = layers;
	inst_info.enabledLayerCount = 1;

	VkResult res = vkCreateInstance( &inst_info, nullptr, &mInstance );
	gDebug() << "vkCreateInstance => " << res;

	if ( res == VK_ERROR_INCOMPATIBLE_DRIVER ) {
		std::cout << "cannot find a compatible Vulkan ICD\n";
		exit( -1 );
	} else if ( res ) {
		std::cout << "unknown error (" << res << ")\n";
		exit( -1 );
	}


	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = &VulkanInstance::debugCallback;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr( mInstance, "vkCreateDebugUtilsMessengerEXT" );
	if ( func == nullptr ) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
	if ( func( mInstance, &createInfo, nullptr, &mDebugMessenger ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to set up debug messenger!");
	}


	EnumerateGpus();
	gDebug() << "VulkanInstance => ok";
}


VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData )
{
	fDebug();
	gDebug() << "validation layer: " << pCallbackData->pMessage;
	void* array[16];
	size_t size;
	size = backtrace( array, 16 );
	char** trace = backtrace_symbols( array, size );
	gDebug() << "Stack trace :";
	for ( size_t i = 0; i < size; i++ ) {
		gDebug() << "    " << trace[i];
	}

	if ( messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ) {
		exit(0);
	}
	return VK_FALSE;
}

static uintptr_t _ge_AllocMemBlock( uintptr_t size )
{
	return (uintptr_t)malloc( size );
}


void* VulkanInstance::sAlloc( size_t size, size_t align, int32_t allocType )
{
	fDebug( size, align, allocType );

	uintptr_t block_sz = sizeof(uintptr_t) * 2;
	size_t fullSize = size + ( align > block_sz ? align : block_sz ) + align;
	uintptr_t addr = _ge_AllocMemBlock( fullSize );
	uintptr_t* var = (uintptr_t*)( ALIGN( addr + ( align > block_sz ? align : block_sz ), align ) - block_sz );
	var[0] = addr;
	var[1] = fullSize;
	memset( (void*)(uintptr_t)&var[2], 0x0, size );
// 	mCpuRamCounter += fullSize; // TODO
	return (void*)(uintptr_t)&var[2];
}


void VulkanInstance::sFree( void* pMem )
{
	fDebug( pMem );

	if ( pMem && pMem != (void*)0xDEADBEEF && pMem != (void*)0xBAADF00D ) {
		uintptr_t* var = (uintptr_t*)pMem;
		uintptr_t addr = var[-2];
		size_t fullSize = var[-1];
		free( (void*)addr );
// 		mCpuRamCounter -= fullSize; // TODO
	}
}


int VulkanInstance::EnumerateGpus()
{
	fDebug();

	uint32_t gpuCount = 0;
	VkResult res = vkEnumeratePhysicalDevices( mInstance, &gpuCount, nullptr );
	gDebug() << "vkEnumeratePhysicalDevices => " << res;
	gDebug() << "Num GPUs : " << gpuCount;
	mGpus.resize( gpuCount );
	mDevices.resize( gpuCount );
	res = vkEnumeratePhysicalDevices( mInstance, &gpuCount, mGpus.data() );
	gDebug() << "vkEnumeratePhysicalDevices => " << res;

	for ( auto gpu : mGpus ) {
		VkPhysicalDeviceProperties props = {};
		vkGetPhysicalDeviceProperties( gpu, &props );
		gDebug() << "gpu " << gpu << " :";
		gDebug() << "    VID : " << std::hex << props.vendorID;
		gDebug() << "    DID : " << std::hex << props.deviceID;
		gDebug() << "    type : " << props.deviceType;
		gDebug() << "    name : " << props.deviceName;
	}

	return gpuCount;
}


int32_t VulkanInstance::findQueueFamilyIndex( std::function< bool( uint32_t, VkQueueFamilyProperties* ) > cb )
{
	int32_t ret = -1;

	for ( unsigned int i = 0; i < mQueueFamilyProperties.size(); i++ ) {
		if ( cb( i, &mQueueFamilyProperties[i] ) == true ) {
			return i;
		}
	}

	return -1;
}


Instance* VulkanInstance::CreateDevice( int devid, int queueCount )
{
	fDebug( devid, queueCount );


	uint32_t qeueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( mGpus[devid], &qeueFamilyCount, nullptr );
	mQueueFamilyProperties.resize( qeueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( mGpus[devid], &qeueFamilyCount, mQueueFamilyProperties.data() );


	VulkanInstance* ret = (VulkanInstance*)malloc( sizeof(VulkanInstance) );
	memset( (void*)ret, 0, sizeof(VulkanInstance) );
	memcpy( (void*)ret, (void*)this, sizeof(VulkanInstance) );
	ret->mCpuRamCounter = 0;
	ret->mGpuRamCounter = 0;
	ret->mDeviceId = devid;
	ret->mBoundFramebuffers = std::map< Thread*, VulkanFramebuffer* >();

	ret->mGraphicsQueueFamilyIndex = findQueueFamilyIndex( []( uint32_t i, VkQueueFamilyProperties* prop ) {
		return ( prop->queueFlags & VK_QUEUE_GRAPHICS_BIT );
	});
	if ( ret->mGraphicsQueueFamilyIndex < 0 ) {
		gDebug() << "ERROR : Cannot find Graphics queue";
		exit(0);
	}

	GE::Window* window = ret->CreateWindow( "", 64, 64, GE::Window::Hidden );

	ret->mPresentationQueueFamilyIndex = findQueueFamilyIndex( [this,devid,window]( uint32_t i, VkQueueFamilyProperties* prop ) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR( mGpus[devid], i, static_cast<VulkanWindow*>(window)->surface(), &presentSupport );
		return presentSupport;
	});
	if ( ret->mPresentationQueueFamilyIndex < 0 ) {
		gDebug() << "ERROR : Cannot find presentation queue";
		exit(0);
	}

	delete window;


	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	const float queue_priority = 1.0 ;
	VkDeviceQueueCreateInfo queue_info = {};
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = ret->mGraphicsQueueFamilyIndex;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = &queue_priority;
	queueCreateInfos.push_back( queue_info );

	if ( ret->mPresentationQueueFamilyIndex != ret->mGraphicsQueueFamilyIndex ) {
		VkDeviceQueueCreateInfo queue_info2 = {};
		queue_info2.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info2.queueFamilyIndex = ret->mPresentationQueueFamilyIndex;
		queue_info2.queueCount = 1;
		queue_info2.pQueuePriorities = &queue_priority;
		queueCreateInfos.push_back( queue_info2 );
	}

	const VkPhysicalDeviceFeatures features = {
		.multiDrawIndirect = true,
		.fillModeNonSolid = true,
		.samplerAnisotropy = true,
	};
	const VkPhysicalDeviceDescriptorIndexingFeaturesEXT featuresEXT = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
		.runtimeDescriptorArray = true,
	};

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = &featuresEXT;
	device_info.queueCreateInfoCount = queueCreateInfos.size();
	device_info.pQueueCreateInfos = queueCreateInfos.data();
	const char* layers[] = { "VK_LAYER_LUNARG_standard_validation", "VK_LAYER_LUNARG_api_dump" };
	device_info.ppEnabledLayerNames = layers;
	device_info.enabledLayerCount = 2;
	device_info.pEnabledFeatures = &features;
    std::vector<const char*> deviceExtensions = { "VK_KHR_swapchain" };
	device_info.ppEnabledExtensionNames = deviceExtensions.data();
	device_info.enabledExtensionCount = static_cast<uint32_t> (deviceExtensions.size());

	VkResult res = vkCreateDevice( ret->mGpus[devid], &device_info, nullptr, &ret->mDevices[devid] );
	gDebug() << "vkCreateDevice => " << res;

	vkGetDeviceQueue( ret->mDevices[ret->mDeviceId], ret->mGraphicsQueueFamilyIndex, 0, &ret->mGraphicsQueue );
	vkGetDeviceQueue( ret->mDevices[ret->mDeviceId], ret->mPresentationQueueFamilyIndex, 0, &ret->mPresentationQueue );
	gDebug() << "mGraphicsQueue => " << ret->mGraphicsQueue;
	gDebug() << "mPresentationQueue => " << ret->mPresentationQueue;




	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = ret->mPresentationQueueFamilyIndex;
	if ( vkCreateCommandPool( ret->mDevices[ret->mDeviceId], &poolInfo, nullptr, &ret->mCommandPool ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create command pool!");
	}
	gDebug() << "mCommandPool => " << ret->mCommandPool;




	ret->mRenderPass = new VulkanRenderPass( ret );
/*

	// Create the command buffer from the command pool
	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = nullptr;
	cmd.commandPool = ret->mCommandPool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	res = vkAllocateCommandBuffers( ret->mDevices[ret->mDeviceId] , &cmd, &ret->mCommandBuffer );
	gDebug() << "vkAllocateCommandBuffers => " << res;
*/



/*
	VK_FENCE_CREATE_INFO fenceCreateInfo = {};
	vkCreateFence( ret->mDevices[devid], &fenceCreateInfo, &ret->mFences[devid] );
*/

	if ( !mBaseInstance || mBaseInstance->device() == 0 ) {
		mBaseInstance = ret;
	}

	return ret;

}


uint32_t VulkanInstance::FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties )
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties( mGpus[mDeviceId], &memProperties );

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}


VkResult VulkanInstance::CreateBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory )
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if ( vkCreateBuffer( mDevices[mDeviceId], &bufferInfo, nullptr, buffer ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements( mDevices[mDeviceId], *buffer, &memRequirements );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType( memRequirements.memoryTypeBits, properties );

	if ( vkAllocateMemory( mDevices[mDeviceId], &allocInfo, nullptr, bufferMemory ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	((VulkanInstance*)Instance::baseInstance())->AffectVRAM( memRequirements.size );

	return vkBindBufferMemory( mDevices[mDeviceId], *buffer, *bufferMemory, 0 );
}


VkResult VulkanInstance::CreateImage( uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory )
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if ( vkCreateImage( mDevices[mDeviceId], &imageInfo, nullptr, image ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements( mDevices[mDeviceId], *image, &memRequirements );

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType( memRequirements.memoryTypeBits, properties );

	if ( vkAllocateMemory( mDevices[mDeviceId], &allocInfo, nullptr, imageMemory ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	((VulkanInstance*)Instance::baseInstance())->AffectVRAM( memRequirements.size );

	vkBindImageMemory( mDevices[mDeviceId], *image, *imageMemory, 0 );
}



VkResult VulkanInstance::CopyBuffer( VkBuffer dstBuffer, VkBuffer srcBuffer, VkDeviceSize offset, VkDeviceSize size )
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers( mDevices[mDeviceId], &allocInfo, &commandBuffer );

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer( commandBuffer, &beginInfo );
		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		copyRegion.srcOffset = offset;
		copyRegion.dstOffset = offset;
		vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );
	vkEndCommandBuffer( commandBuffer );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit( mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( mGraphicsQueue );
	vkFreeCommandBuffers( mDevices[mDeviceId], mCommandPool, 1, &commandBuffer );
	return VK_SUCCESS;
}


VkResult VulkanInstance::CopyBufferToImage( VkImage image, VkBuffer buffer, uint32_t width, uint32_t height )
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers( mDevices[mDeviceId], &allocInfo, &commandBuffer );

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer( commandBuffer, &beginInfo );
	vkCmdCopyBufferToImage( commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
	vkEndCommandBuffer( commandBuffer );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit( mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( mGraphicsQueue );
	vkFreeCommandBuffers( mDevices[mDeviceId], mCommandPool, 1, &commandBuffer );
	return VK_SUCCESS;
}


void VulkanInstance::TransitionImageLayout( VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout )
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers( mDevices[mDeviceId], &allocInfo, &commandBuffer );
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer( commandBuffer, &beginInfo );

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = aspect;

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else if ( oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL and newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ) {
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier( commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier );

	vkEndCommandBuffer( commandBuffer );
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit( mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( mGraphicsQueue );

	vkFreeCommandBuffers( mDevices[mDeviceId], mCommandPool, 1, &commandBuffer );
}


uint64_t VulkanInstance::ReferenceImage( Image* image )
{
	fDebug( image, image->width(), image->height() );
	VulkanTexture* ret = new VulkanTexture( this, image );
	return reinterpret_cast< uint64_t >( ret );
}


void VulkanInstance::UnreferenceImage( uint64_t ref )
{
	VulkanTexture* tex = reinterpret_cast< VulkanTexture* >( ref );
	delete tex;
}


void VulkanInstance::UpdateImageData( Image* image, uint64_t ref )
{
}


void VulkanInstance::AffectVRAM( int64_t sz )
{
	mGpuRamCounter += sz;
}


VulkanTexture::VulkanTexture( VulkanInstance* instance, Image* image )
	: mGeImage( image )
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkDeviceSize imageSize = image->width() * image->height() * 4;

	instance->CreateBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory );

	void* data;
	vkMapMemory( instance->device(), stagingBufferMemory, 0, imageSize, 0, &data );
		memcpy( data, image->data(), static_cast<size_t>(imageSize) );
	vkUnmapMemory( instance->device(), stagingBufferMemory );

	instance->CreateImage( image->width(), image->height(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mImage, &mImageMemory );

	instance->TransitionImageLayout( mImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
		instance->CopyBufferToImage( mImage, stagingBuffer, static_cast<uint32_t>(image->width()), static_cast<uint32_t>(image->height()));
	instance->TransitionImageLayout( mImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

	vkDestroyBuffer( instance->device(), stagingBuffer, nullptr );
	vkFreeMemory( instance->device(), stagingBufferMemory, nullptr );



	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = mImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	if ( vkCreateImageView( instance->device(), &viewInfo, nullptr, &mImageView ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create texture image view!");
	}



	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	if ( vkCreateSampler( instance->device(), &samplerInfo, nullptr, &mSampler ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}
