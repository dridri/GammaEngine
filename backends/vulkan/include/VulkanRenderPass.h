#ifndef VULKANRENDERPASS_H
#define VULKANRENDERPASS_H

#include "VulkanInstance.h"

namespace GE {

class VulkanRenderPass
{
public:
	VulkanRenderPass( VulkanInstance* instance, uint32_t colorAttachmentCount = 1, uint32_t depthAttachmentCount = 1 );
	~VulkanRenderPass();

	VkRenderPass& renderPass() { return mRenderPass; }

protected:
	VulkanInstance* mInstance;
	VkRenderPass mRenderPass;
	uint32_t mColorAttachmentCount;
	uint32_t mDepthAttachmentCount;
};

} // namespace GE

#endif // VULKANRENDERPASS_H
