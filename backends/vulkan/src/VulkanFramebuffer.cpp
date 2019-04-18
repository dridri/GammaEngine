#include "VulkanFramebuffer.h"
#include "VulkanInstance.h"

using namespace GE;

uint64_t counter = 0;

VulkanFramebuffer::VulkanFramebuffer( VulkanInstance* inst, VkFramebuffer fb, VkViewport viewport, VkRect2D scissors, VkFence fence, VkSemaphore imgAvailSema )
 : mInstance( inst )
 , mFramebuffer( fb )
 , mHash( (uintptr_t)this * (uintptr_t)fb + ( counter++ ) )
 , mViewport( viewport )
 , mScissors( scissors )
 , mFence( fence )
 , mImageAvailableSemaphore( imgAvailSema )
{
}


VulkanFramebuffer::~VulkanFramebuffer()
{
	if ( mFramebuffer ) {
		vkDestroyFramebuffer( mInstance->device(), mFramebuffer, nullptr );
	}
}


void GE::VulkanFramebuffer::waitFence()
{
	vkWaitForFences( mInstance->device(), 1, &mFence, VK_TRUE, UINT64_MAX );
	vkResetFences( mInstance->device(), 1, &mFence );
}
