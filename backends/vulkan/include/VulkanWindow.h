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

#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include <string>

#include <vulkan/vulkan.h>
#include "Window.h"
#include "BaseWindow.h"
#include "VulkanFramebuffer.h"

namespace GE {

class Instance;

class VulkanWindow : public GE::Window
{
public:
	VulkanWindow( Instance* instance, const std::string& title, int width, int height, Flags flags );
	~VulkanWindow();

	virtual void Clear( uint32_t color = 0xFF000000 );
	virtual void BindTarget();
	virtual void SwapBuffers();

	virtual uint64_t colorImage();

	virtual void ReadKeys( bool* keys );
	virtual uint64_t CreateSharedContext();
	virtual void BindSharedContext( uint64_t ctx );

	VkSurfaceKHR surface() const { return mSurface; }
	const std::vector<VulkanFramebuffer*>& framebuffers() const { return mSwapChainFramebuffers; }

private:
	typedef struct {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	} SwapChainSupportDetails;
	SwapChainSupportDetails querySwapChainSupport( VkPhysicalDevice device );

	void InitPresentableImage();
	void createRenderPass();
	void createClearCommandBuffers();

	VkSurfaceKHR mSurface;
	VkSurfaceFormatKHR mSurfaceFormat;
	VkExtent2D mSwapChainExtent;
	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mSwapChainImageViews;
	VkImage mDepthImage;
	VkDeviceMemory mDepthImageMemory;
	VkImageView mDepthImageView;
// 	VkRenderPass mRenderPass;
	std::vector<VulkanFramebuffer*> mSwapChainFramebuffers;

	VkSemaphore mImageAvailableSemaphores[2];
// 	VkSemaphore mRenderFinishedSemaphores[2];
	VkFence mInFlightFences[2];

	uint32_t mBackImageIndex;
	std::vector< VkCommandBuffer > mClearCommandBuffers;
	uint32_t mClearColor;
};

}; // namespace GE

#endif // VULKANWINDOW_H
 
