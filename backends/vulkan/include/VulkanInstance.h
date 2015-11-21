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
#include "Instance.h"
#include "vulkan.h"

namespace GE {
	class Window;
	class Renderer;
	class Vertex;
	class Object;
}
using namespace GE;

class VulkanInstance : public Instance
{
public:
	VulkanInstance( const char* appName, uint32_t appVersion );
	virtual ~VulkanInstance(){}

	virtual int EnumerateGpus();
	virtual Instance* CreateDevice( int devid, int queueCount = 1 );
	virtual uint64_t ReferenceImage( Image* image );
	virtual void UnreferenceImage( uint64_t ref );

	VK_MEMORY_REF AllocateObject( VK_OBJECT object );
	VK_MEMORY_REF AllocateMappableBuffer( size_t size );
	static void UpdateDescriptorSet( VK_DESCRIPTOR_SET descriptorSet, VK_MEMORY_VIEW_ATTACH_INFO* memoryViewAttachInfo );

	void QueueSubmit( VK_CMD_BUFFER buf, VK_MEMORY_REF* refs, int nRefs );

private:
	static void* sAlloc( size_t size, size_t align, int32_t allocType );
	static void sFree( void* pMem );

	VK_APPLICATION_INFO mAppInfo;
	VK_ALLOC_CALLBACKS mAllocCb;
	VK_INSTANCE mInstance;
};

#endif // VULKANINSTANCE_H
