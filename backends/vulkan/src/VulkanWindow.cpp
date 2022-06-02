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
 * This provkam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This provkam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this provkam.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <unistd.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include "VulkanInstance.h"
#include "VulkanWindow.h"
#include "Debug.h"
#include "VulkanRenderPass.h"

extern "C" GE::Window* CreateWindow( GE::Instance* instance, const std::string& title, int width, int height, VulkanWindow::Flags flags ) {
	return new VulkanWindow( instance, title, width, height, flags );
}

VulkanWindow::VulkanWindow( Instance* instance, const std::string& title, int width, int height, Flags flags )
	: Window( instance, title, width, height, flags )
// 	, mColorImage( 0 )
// 	, mColorImageMemRef( {} )
// 	, mImageColorRange( {} )
// 	, mColorTargetView( 0 )
// 	, mDepthImage( 0 )
// 	, mDepthImageMemRef( {} )
// 	, mImageDepthRange( {} )
// 	, mDepthTargetView( 0 )
// 	, mBindCmdBuffer( 0 )
// 	, mClearCmdBuffer( 0 )
	, mSurface( 0 )
	, mSwapChain( 0 )
	, mImageAvailableSemaphores { 0, 0, 0, 0 }
	, mInFlightFences { 0, 0, 0, 0 }
	, mCurrentFrame( 0 )
	, mClearColor( 0 )
{
#ifdef GE_WIN32
#elif defined(GE_LINUX)
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.dpy = static_cast< _XDisplay* >( mDisplay );
	surfaceCreateInfo.window = mWindow;
	VkResult res = vkCreateXlibSurfaceKHR( static_cast< VulkanInstance* >( mInstance )->instance(), &surfaceCreateInfo, nullptr, &mSurface );
	gDebug() << "vkCreateXlibSurfaceKHR : " << res;
	gDebug() << "mSurface : " << mSurface;

	int32_t ret = static_cast< VulkanInstance* >( mInstance )->findQueueFamilyIndex( [this]( uint32_t i, VkQueueFamilyProperties* prop ) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR( static_cast< VulkanInstance* >( mInstance )->gpu(), i, mSurface, &presentSupport );
		return presentSupport;
	});
	if ( ret < 0 or (uint32_t)ret != static_cast< VulkanInstance* >( mInstance )->presentationQueueFamilyIndex() ) {
		gDebug() << "presentationQueueFamilyIndex missmatch for Window { " << title << ", " << width << ", " << height << " }";
		exit(0);
	}
#endif

	if ( ( flags & Window::Hidden ) == false ) {
		InitPresentableImage();
		createClearCommandBuffers();
		vkAcquireNextImageKHR( static_cast< VulkanInstance* >( mInstance )->device(), mSwapChain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &mBackImageIndex );
	}
}


VulkanWindow::~VulkanWindow()
{
#ifdef GE_WIN32
#elif defined(GE_LINUX)
	if ( mSurface ) {
		VulkanInstance* instance = static_cast< VulkanInstance* >( mInstance );
		vkDestroySurfaceKHR( instance->instance(), mSurface, nullptr );
	}
#endif
}


uint64_t VulkanWindow::colorImage()
{
	return 0;
}

/*
VK_IMAGE VulkanWindow::colorImage()
{
	return mColorImage;
}
*/



VulkanWindow::SwapChainSupportDetails VulkanWindow::querySwapChainSupport( VkPhysicalDevice device )
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, mSurface, &details.capabilities );

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR( device, mSurface, &formatCount, nullptr );

	if ( formatCount != 0 ) {
		details.formats.resize( formatCount );
		vkGetPhysicalDeviceSurfaceFormatsKHR( device, mSurface, &formatCount, details.formats.data() );
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR( device, mSurface, &presentModeCount, nullptr );

	if ( presentModeCount != 0 ) {
		details.presentModes.resize( presentModeCount );
		vkGetPhysicalDeviceSurfacePresentModesKHR( device, mSurface, &presentModeCount, details.presentModes.data() );
	}

	return details;
}


void VulkanWindow::InitPresentableImage()
{
	VulkanInstance* instance = static_cast< VulkanInstance* >( mInstance );
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport( instance->gpu() );

	// Destroy previous context
	if ( mSwapChainImageViews.size() > 0 ) {
		vkWaitForFences( instance->device(), mSwapChainImageViews.size(), mInFlightFences, VK_TRUE, UINT64_MAX );
		vkQueueWaitIdle( instance->graphicsQueue() );
		vkQueueWaitIdle( instance->presentationQueue() );
		vkDeviceWaitIdle( instance->device() );
		/*
		for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
			vkDestroySemaphore( instance->device(), mImageAvailableSemaphores[i], nullptr );
			vkDestroyFence( instance->device(), mInFlightFences[i], nullptr );
			mImageAvailableSemaphores[i] = nullptr;
			mInFlightFences[i] = nullptr;
		}
		*/
		vkDestroyImageView( instance->device(), mDepthImageView, nullptr );
		vkFreeMemory( instance->device(), mDepthImageMemory, nullptr );
		vkDestroyImage( instance->device(), mDepthImage, nullptr );
		mDepthImageView = nullptr;
		mDepthImageMemory = nullptr;
		mDepthImage = nullptr;
		for ( auto img : mSwapChainImageViews ) {
			vkDestroyImageView( instance->device(), img, nullptr );
		}
// 		vkDestroySwapchainKHR( instance->device(), mSwapChain, nullptr );
// 		mSwapChain = nullptr;
	}


	mSurfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
/*
	if ( swapChainSupport.formats.size() == 1 && swapChainSupport.formats[0].format == VK_FORMAT_UNDEFINED ) {
		mSurfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	} else {
		bool ok = false;
		for ( const auto& availableFormat : swapChainSupport.formats ) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				mSurfaceFormat = availableFormat;
				ok = true;
				break;
			}
		}
		if ( not ok ) {
			mSurfaceFormat = swapChainSupport.formats[0];
		}
	}
*/
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for ( const auto& availablePresentMode : swapChainSupport.presentModes ) {
		if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR ) {
			presentMode = availablePresentMode;
			break;
		} else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR ) {
			presentMode = availablePresentMode;
			break;
		}
	}

	if ( swapChainSupport.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() ) {
		mSwapChainExtent = swapChainSupport.capabilities.currentExtent;
	} else {
		mSwapChainExtent = { mWidth, mHeight };
		mSwapChainExtent.width = std::max( swapChainSupport.capabilities.minImageExtent.width, std::min( swapChainSupport.capabilities.maxImageExtent.width, mSwapChainExtent.width ) );
		mSwapChainExtent.height = std::max( swapChainSupport.capabilities.minImageExtent.height, std::min( swapChainSupport.capabilities.maxImageExtent.height, mSwapChainExtent.height ) );
	}


	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface;

	createInfo.minImageCount = 3;
	createInfo.imageFormat = mSurfaceFormat.format;
	createInfo.imageColorSpace = mSurfaceFormat.colorSpace;
	createInfo.imageExtent = mSwapChainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	uint32_t queueFamilyIndices[] = { instance->graphicsQueueFamilyIndex(), instance->presentationQueueFamilyIndex() };
	if ( queueFamilyIndices[0] != queueFamilyIndices[1] ) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = mSwapChain;
	if ( vkCreateSwapchainKHR( instance->device(), &createInfo, nullptr, &mSwapChain ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create swap chain!");
	}
	if ( createInfo.oldSwapchain ) {
		vkDestroySwapchainKHR( instance->device(), createInfo.oldSwapchain, nullptr );
	}

	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR( instance->device(), mSwapChain, &imageCount, nullptr );
	mSwapChainImages.resize( imageCount );
	vkGetSwapchainImagesKHR( instance->device(), mSwapChain, &imageCount, mSwapChainImages.data() );

	gDebug() << "SwapChain images count : " << mSwapChainImages.size();


	mSwapChainImageViews.resize( mSwapChainImages.size() );
	for ( uint32_t i = 0; i < mSwapChainImages.size(); i++ ) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = mSwapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = mSurfaceFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if ( vkCreateImageView( instance->device(), &createInfo, nullptr, &mSwapChainImageViews[i] ) != VK_SUCCESS ) {
			throw std::runtime_error("failed to create image views!");
		}
	}

	VkImageCreateInfo depthImageInfo = {};
	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
	depthImageInfo.extent.width = mSwapChainExtent.width;
	depthImageInfo.extent.height = mSwapChainExtent.height;
	depthImageInfo.extent.depth = 1;
	depthImageInfo.mipLevels = 1;
	depthImageInfo.arrayLayers = 1;
	depthImageInfo.format = VK_FORMAT_D32_SFLOAT;
	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkCreateImage( instance->device(), &depthImageInfo, nullptr, &mDepthImage );

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements( instance->device(), mDepthImage, &memRequirements );
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = instance->FindMemoryType( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
	if ( vkAllocateMemory( instance->device(), &allocInfo, nullptr, &mDepthImageMemory ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to allocate image memory!");
	}
	((VulkanInstance*)Instance::baseInstance())->AffectVRAM( memRequirements.size );
	vkBindImageMemory( instance->device(), mDepthImage, mDepthImageMemory, 0 );

	VkImageViewCreateInfo depthViewCreateInfo = {};
	depthViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthViewCreateInfo.image = mDepthImage;
	depthViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthViewCreateInfo.format = VK_FORMAT_D32_SFLOAT;
	depthViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthViewCreateInfo.subresourceRange.baseMipLevel = 0;
	depthViewCreateInfo.subresourceRange.levelCount = 1;
	depthViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	depthViewCreateInfo.subresourceRange.layerCount = 1;
	if ( vkCreateImageView( instance->device(), &depthViewCreateInfo, nullptr, &mDepthImageView ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create image views!");
	}

	instance->TransitionImageLayout( mDepthImage, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );



	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
// 	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)mSwapChainExtent.width,
		.height = (float)mSwapChainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	VkRect2D scissors = {
		{ 0, 0 },
		{ mSwapChainExtent.width, mSwapChainExtent.height }
	};

	for ( auto fb : mSwapChainFramebuffers ) {
	//	delete fb;
	}
	if ( mSwapChainFramebuffers.size() > mSwapChainImageViews.size() ) {
		// TODO : notify uses of framebuffers >= mSwapChainImageViews.size() that they have to be destroyed
	}
	mSwapChainFramebuffers.resize( mSwapChainImageViews.size() );
	for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
		if ( mInFlightFences[i] == 0 ) {
			if ( i > 0 ) {
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			}
			if ( vkCreateFence( instance->device(), &fenceInfo, nullptr, &mInFlightFences[i] ) != VK_SUCCESS ) {
				throw std::runtime_error("failed to create fence object for a frame!");
			} else {
				gDebug() << "created fence " << i;
			}
		}
		if ( mImageAvailableSemaphores[i] == 0 ) {
			if ( vkCreateSemaphore( instance->device(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i] ) != VK_SUCCESS ) {
				throw std::runtime_error("failed to create semaphore object for a frame!");
			} else {
				gDebug() << "created sema " << i;
			}
		}

		VkImageView attachments[] = { mSwapChainImageViews[i], mDepthImageView };
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = instance->renderPass()->renderPass();
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = mSwapChainExtent.width;
		framebufferInfo.height = mSwapChainExtent.height;
		framebufferInfo.layers = 1;

		VkFramebuffer fb;
		if ( vkCreateFramebuffer( instance->device(), &framebufferInfo, nullptr, &fb ) != VK_SUCCESS ) {
			throw std::runtime_error("failed to create framebuffer!");
		}
		if ( mSwapChainFramebuffers[i] ) {
			gDebug() << "update fb from " << mSwapChainFramebuffers[i]->framebuffer();
			if ( mSwapChainFramebuffers[i]->framebuffer() ) {
// 				vkDestroyFramebuffer( instance->device(), mSwapChainFramebuffers[i]->framebuffer(), nullptr );
			}
			VulkanFramebuffer* next = new VulkanFramebuffer( static_cast<VulkanInstance*>(mInstance), fb, viewport, scissors, mInFlightFences[i], mImageAvailableSemaphores[i] );
			*mSwapChainFramebuffers[i] = *next;
			free( next );
			gDebug() << "updated fb " << mSwapChainFramebuffers[i] << ", " << mSwapChainFramebuffers[i]->hash();
			gDebug() << "update fb to " << mSwapChainFramebuffers[i]->framebuffer();
		} else {
			mSwapChainFramebuffers[i] = new VulkanFramebuffer( static_cast<VulkanInstance*>(mInstance), fb, viewport, scissors, mInFlightFences[i], mImageAvailableSemaphores[i] );
			gDebug() << "created fb " << mSwapChainFramebuffers[i] << ", " << mSwapChainFramebuffers[i]->hash();
		}
	}
	gDebug() << "Viewport : " << mWidth << " x " << mHeight;
	gDebug() << "Extent : " << mSwapChainExtent.width << " x " << mSwapChainExtent.height;
}


void VulkanWindow::createClearCommandBuffers()
{
	VulkanInstance* instance = static_cast< VulkanInstance* >( mInstance );

	if ( mClearCommandBuffers.size() == 0 ) {
		VkCommandPool commandPool = instance->commandPool();
		mClearCommandBuffers.resize( mSwapChainImages.size() );
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		int i = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		allocInfo.commandBufferCount = (uint32_t) mSwapChainImages.size();
		if ( vkAllocateCommandBuffers( instance->device(), &allocInfo, mClearCommandBuffers.data() ) != VK_SUCCESS) {
			std::cerr << "failed to allocate presentation command buffers" << std::endl;
			exit(1);
		} else {
			std::cout << "allocated presentation command buffers" << std::endl;
		}
	}

	// Prepare data for recording command buffers
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	// Note: contains value for each subresource range
	VkClearColorValue clearColor = {
		{
			( (float)( mClearColor & 0xFF ) / 255.0f),
			( (float)( ( mClearColor >> 8 ) & 0xFF ) / 255.0f),
			( (float)( ( mClearColor >> 16 ) & 0xFF ) / 255.0f),
			1.0f
		} // R, G, B, A
	};

	VkImageSubresourceRange subResourceRange = {};
	subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResourceRange.baseMipLevel = 0;
	subResourceRange.levelCount = 1;
	subResourceRange.baseArrayLayer = 0;
	subResourceRange.layerCount = 1;

	VkImageSubresourceRange depthSubResourceRange = {};
	depthSubResourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthSubResourceRange.baseMipLevel = 0;
	depthSubResourceRange.levelCount = 1;
	depthSubResourceRange.baseArrayLayer = 0;
	depthSubResourceRange.layerCount = 1;

	static const VkClearDepthStencilValue clearDepth = {
		.depth = 1.0f,
		.stencil = 0
	};

	// Record the command buffer for every swap chain image
	for ( uint32_t i = 0; i < mSwapChainImages.size(); i++ ) {
		vkResetCommandBuffer( mClearCommandBuffers[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );

		// Change layout of image to be optimal for clearing
		// Note: previous layout doesn't matter, which will likely cause contents to be discarded
		VkImageMemoryBarrier presentToClearBarrier_color = {};
		presentToClearBarrier_color.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		presentToClearBarrier_color.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		presentToClearBarrier_color.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		presentToClearBarrier_color.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		presentToClearBarrier_color.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		presentToClearBarrier_color.srcQueueFamilyIndex = instance->presentationQueueFamilyIndex();
		presentToClearBarrier_color.dstQueueFamilyIndex = instance->presentationQueueFamilyIndex();
		presentToClearBarrier_color.image = mSwapChainImages[i];
		presentToClearBarrier_color.subresourceRange = subResourceRange;

		// Change layout of image to be optimal for presenting
		VkImageMemoryBarrier clearToPresentBarrier_color = {};
		clearToPresentBarrier_color.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		clearToPresentBarrier_color.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		clearToPresentBarrier_color.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		clearToPresentBarrier_color.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		clearToPresentBarrier_color.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		clearToPresentBarrier_color.srcQueueFamilyIndex = instance->presentationQueueFamilyIndex();
		clearToPresentBarrier_color.dstQueueFamilyIndex = instance->presentationQueueFamilyIndex();
		clearToPresentBarrier_color.image = mSwapChainImages[i];
		clearToPresentBarrier_color.subresourceRange = subResourceRange;

		// Change layout of depth image to be optimal for clearing
		// Note: previous layout doesn't matter, which will likely cause contents to be discarded
		VkImageMemoryBarrier presentToClearBarrier_depth = {};
		presentToClearBarrier_depth.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		presentToClearBarrier_depth.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		presentToClearBarrier_depth.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		presentToClearBarrier_depth.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		presentToClearBarrier_depth.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		presentToClearBarrier_depth.srcQueueFamilyIndex = instance->presentationQueueFamilyIndex();
		presentToClearBarrier_depth.dstQueueFamilyIndex = instance->presentationQueueFamilyIndex();
		presentToClearBarrier_depth.image = mDepthImage;
		presentToClearBarrier_depth.subresourceRange = depthSubResourceRange;

		// Change layout of image to be optimal for presenting
		VkImageMemoryBarrier clearToPresentBarrier_depth = {};
		clearToPresentBarrier_depth.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		clearToPresentBarrier_depth.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		clearToPresentBarrier_depth.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		clearToPresentBarrier_depth.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		clearToPresentBarrier_depth.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		clearToPresentBarrier_depth.srcQueueFamilyIndex = instance->presentationQueueFamilyIndex();
		clearToPresentBarrier_depth.dstQueueFamilyIndex = instance->presentationQueueFamilyIndex();
		clearToPresentBarrier_depth.image = mDepthImage;
		clearToPresentBarrier_depth.subresourceRange = depthSubResourceRange;

		// Record command buffer
		vkBeginCommandBuffer( mClearCommandBuffers[i], &beginInfo );

			vkCmdPipelineBarrier( mClearCommandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToClearBarrier_color );
			vkCmdClearColorImage( mClearCommandBuffers[i], mSwapChainImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subResourceRange );
			vkCmdPipelineBarrier( mClearCommandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier_color );

			vkCmdPipelineBarrier( mClearCommandBuffers[i], VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToClearBarrier_depth );
			vkCmdClearDepthStencilImage( mClearCommandBuffers[i], mDepthImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepth, 1, &depthSubResourceRange );
			vkCmdPipelineBarrier( mClearCommandBuffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier_depth );

		if ( vkEndCommandBuffer( mClearCommandBuffers[i] ) != VK_SUCCESS ) {
			std::cerr << "failed to record command buffer" << std::endl;
			exit(1);
		}
	}
}


void VulkanWindow::Clear( uint32_t color )
{
	VulkanInstance* instance = static_cast< VulkanInstance* >( mInstance );

	if ( color != mClearColor ) {
		mClearColor = color;
		createClearCommandBuffers();
	}

	VkSubmitInfo submitInfo = {};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &mImageAvailableSemaphores[mCurrentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = &mImageAvailableSemaphores[mCurrentFrame];
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mClearCommandBuffers[mBackImageIndex];

// 	vkWaitForFences( static_cast< VulkanInstance* >( mInstance )->device(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX );
	vkResetFences( static_cast< VulkanInstance* >( mInstance )->device(), 1, &mInFlightFences[mCurrentFrame] );

	if ( instance->graphicsQueueSubmit( 1, &submitInfo, mInFlightFences[mCurrentFrame] ) != VK_SUCCESS ) {
		std::cerr << "failed to submit draw command buffer" << std::endl;
		exit(1);
	}
}


void VulkanWindow::BindTarget()
{
	static_cast< VulkanInstance* >( mInstance )->boundFramebuffers()[Thread::currentThread()] = mSwapChainFramebuffers[mBackImageIndex];
}


void VulkanWindow::SwapBuffers()
{
	VulkanInstance* instance = static_cast< VulkanInstance* >( mInstance );


	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 0;
	presentInfo.pWaitSemaphores = &mImageAvailableSemaphores[mCurrentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mSwapChain;
	presentInfo.pImageIndices = &mBackImageIndex;
	VkResult result = vkQueuePresentKHR( instance->presentationQueue(), &presentInfo );


	SwapBuffersBase();

	if ( result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR or mHasResized ) {
		mHasResized = false;
		InitPresentableImage();
		createClearCommandBuffers();
	}

	mCurrentFrame = ( mCurrentFrame + 1 ) % 3;
	vkWaitForFences( static_cast< VulkanInstance* >( mInstance )->device(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX );
	vkAcquireNextImageKHR( static_cast< VulkanInstance* >( mInstance )->device(), mSwapChain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &mBackImageIndex );
}


uint64_t VulkanWindow::CreateSharedContext()
{
	return 0;
}


void VulkanWindow::BindSharedContext( uint64_t ctx )
{
}


void VulkanWindow::ReadKeys( bool* keys )
{
	memcpy( keys, mKeys, sizeof( mKeys ) );
}
