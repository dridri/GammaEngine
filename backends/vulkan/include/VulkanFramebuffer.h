#ifndef VULKANFRAMEBUFFER_H
#define VULKANFRAMEBUFFER_H

#include "VulkanInstance.h"

namespace GE {

class VulkanFramebuffer
{
public:
	VulkanFramebuffer( VulkanInstance* inst, VkFramebuffer fb = VK_NULL_HANDLE, VkViewport viewport = {}, VkRect2D scissors = {}, VkFence fence = VK_NULL_HANDLE, VkSemaphore imgAvailSema = VK_NULL_HANDLE );
	~VulkanFramebuffer();

	uint64_t hash() const { return mHash; }
	VkFramebuffer framebuffer() const { return mFramebuffer; }
	VkViewport& viewport() { return mViewport; }
	VkRect2D& scissors() { return mScissors; }
	VkFence& fence() { return mFence; }
	VkSemaphore& imageAvailableSemaphores() { return mImageAvailableSemaphore; }

	void waitFence();

private:
	VulkanInstance* mInstance;
	VkFramebuffer mFramebuffer;
	uint64_t mHash;
	VkViewport mViewport;
	VkRect2D mScissors;
	VkFence mFence;
	VkSemaphore mImageAvailableSemaphore;
};

} // namespace GE

#endif // VULKANFRAMEBUFFER_H
