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

#include "VulkanInstance.h"
#include "Vertex.h"
#include "Debug.h"

#ifndef ALIGN
	#define ALIGN(x, align) (((x)+((align)-1))&~((align)-1))
#endif

extern "C" GE::Instance* CreateInstance( const char* appName, uint32_t appVersion ) {
	return new VulkanInstance( appName, appVersion );
}


VulkanInstance::VulkanInstance( const char* appName, uint32_t appVersion )
	: Instance()
{
	fDebug( appName, appVersion );

// 	if ( !mBaseInstance || mBaseInstance->device() == 0 ) {
// 		mBaseInstance = this;
// 	}


	mAppInfo.pAppName = appName;
	mAppInfo.appVersion = appVersion;
	mAppInfo.pEngineName = "Gamma Engine";
	mAppInfo.engineVersion = 0x00020000;
	mAppInfo.apiVersion = VK_API_VERSION;

	mAllocCb.pfnAlloc = &VulkanInstance::sAlloc;
	mAllocCb.pfnFree = &VulkanInstance::sFree;

	vkCreateInstance( &mAppInfo, &mAllocCb, &mInstance );
	EnumerateGpus();
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
	fDebug0();

	vkEnumerateGpus( mInstance, sizeof(mGpus) / sizeof(mGpus[0]), &mGpuCount, mGpus );
	return mGpuCount;
}


Instance* VulkanInstance::CreateDevice( int devid, int queueCount )
{
	VulkanInstance* ret = (VulkanInstance*)malloc( sizeof(VulkanInstance) );
	memset( ret, 0, sizeof(VulkanInstance) );
	memcpy( ret, this, sizeof(VulkanInstance) );
	ret->mCpuRamCounter = 0;
	ret->mGpuRamCounter = 0;
	ret->mDevId = devid;

	fDebug( devid, queueCount );

	VK_DEVICE_QUEUE_CREATE_INFO queueInfo = {};
	queueInfo.queueType = 0;
	queueInfo.queueCount = queueCount;

	VK_DEVICE_CREATE_INFO deviceInfo = {};
	deviceInfo.queueRecordCount = 1;
	deviceInfo.pRequestedQueues = &queueInfo;
	deviceInfo.extensionCount = 0; // TODO
	deviceInfo.ppEnabledExtensionNames = nullptr; // TODO
	deviceInfo.flags |= VK_DEVICE_CREATE_VALIDATION;
	deviceInfo.maxValidationLevel = VK_VALIDATION_LEVEL_4;

	vkCreateDevice( ret->mGpus[devid], &deviceInfo, &ret->mDevices[devid] );

	vkGetDeviceQueue( ret->mDevices[devid], 0, 0, &ret->mQueues[devid] );

	VK_FENCE_CREATE_INFO fenceCreateInfo = {};
	vkCreateFence( ret->mDevices[devid], &fenceCreateInfo, &ret->mFences[devid] );

// 	if ( !mBaseInstance || mBaseInstance->device() == 0 ) {
// 		mBaseInstance = ret;
// 	}

	return ret;
}


VK_MEMORY_REF VulkanInstance::AllocateObject( VK_OBJECT object )
{
	VK_MEMORY_REF ret = {};

	VK_MEMORY_REQUIREMENTS memReqs = {};
	VK_SIZE memReqsSize = sizeof(memReqs);
	vkGetObjectInfo( object, VK_INFO_TYPE_MEMORY_REQUIREMENTS, &memReqsSize, &memReqs );

	VK_MEMORY_HEAP_PROPERTIES heapProps = {};
	VK_SIZE heapPropsSize = sizeof(heapProps);
	vkGetMemoryHeapInfo( mDevices[mDevId], memReqs.heaps[0], VK_INFO_TYPE_MEMORY_HEAP_PROPERTIES, &heapPropsSize, &heapProps );

	if ( heapProps.pageSize <= 0 ) {
		VK_MEMORY_REF ret = { 0 };
		return ret;
	}

	VK_MEMORY_ALLOC_INFO allocInfo = {};
	VK_GPU_MEMORY memory;
	allocInfo.size = ( 1 + memReqs.size / heapProps.pageSize ) * heapProps.pageSize;
	allocInfo.alignment = 0; // TESTING/TODO : 16/32/64 perf improv ??
	allocInfo.memPriority = VK_MEMORY_PRIORITY_HIGH;
	allocInfo.heapCount = 1;
	allocInfo.heaps[0] = memReqs.heaps[0];
	vkAllocMemory( mDevices[mDevId], &allocInfo, &memory );

	vkBindObjectMemory( object, memory, 0 );
	ret.mem = memory;

	return ret;
}


VK_MEMORY_REF VulkanInstance::AllocateMappableBuffer( size_t size )
{
	VK_MEMORY_REF ret = {};

	// Find CPU visible (mappable) heap
	VK_UINT heapCount;
	vkGetMemoryHeapCount( mDevices[mDevId], &heapCount );
	VK_MEMORY_HEAP_PROPERTIES heapProps = {};
	VK_SIZE heapPropsSize = sizeof( heapProps );
	VK_UINT suitableHeap = -1;
	for ( VK_UINT i = 0; i < heapCount; i++ ) {
		vkGetMemoryHeapInfo( mDevices[mDevId], i, VK_INFO_TYPE_MEMORY_HEAP_PROPERTIES, &heapPropsSize, &heapProps );
		if ( heapProps.flags & VK_MEMORY_HEAP_CPU_VISIBLE ) {
			suitableHeap = i;
			break;
		}
	}


	if ( heapProps.pageSize <= 0 ) {
		ret = { 0 };
		return ret;
	}

	VK_MEMORY_ALLOC_INFO allocInfo = {};
	VK_GPU_MEMORY memory;
	allocInfo.size = ( 1 + size / heapProps.pageSize ) * heapProps.pageSize;
	allocInfo.alignment = 0;
	allocInfo.memPriority = VK_MEMORY_PRIORITY_HIGH;
	allocInfo.heapCount = 1;
	allocInfo.heaps[0] = suitableHeap;
	vkAllocMemory( mDevices[mDevId], &allocInfo, &memory );

	ret.mem = memory;
	return ret;
}


void VulkanInstance::QueueSubmit( VK_CMD_BUFFER buf, VK_MEMORY_REF* refs, int nRefs )
{
	vkQueueSubmit( mQueues[mDevId], 1, &buf, nRefs, refs, mFences[mDevId] );

// 	vkWaitForFences( mDevices[mDevId], 1, &mFences[mDevId], true, 0.0f ); // TESTING
}


void VulkanInstance::UpdateDescriptorSet( VK_DESCRIPTOR_SET descriptorSet, VK_MEMORY_VIEW_ATTACH_INFO* memoryViewAttachInfo )
{
	memoryViewAttachInfo->stride = sizeof( Vertex );

	// U, V, W, align1
	memoryViewAttachInfo->offset = 0;
	memoryViewAttachInfo->format.channelFormat = VK_CH_FMT_R32G32B32A32;
	memoryViewAttachInfo->format.numericFormat = VK_NUM_FMT_FLOAT;
	vkAttachMemoryViewDescriptors( descriptorSet, 0, 1, memoryViewAttachInfo );

	// COLOR
	memoryViewAttachInfo->offset = sizeof( float ) * 4;
// 		memoryViewAttachInfo->format.channelFormat = VK_CH_FMT_R8G8B8A8;
// 		memoryViewAttachInfo->format.numericFormat = VK_NUM_FMT_SRGB;
	memoryViewAttachInfo->format.channelFormat = VK_CH_FMT_R32G32B32A32;
	memoryViewAttachInfo->format.numericFormat = VK_NUM_FMT_FLOAT;
	vkAttachMemoryViewDescriptors( descriptorSet, 1, 1, memoryViewAttachInfo );

	// NX, NY, NZ, align2
// 		memoryViewAttachInfo->offset = sizeof( float ) * 4 + sizeof( uint32_t );
	memoryViewAttachInfo->offset = sizeof( float ) * 4 + sizeof( float ) * 4;
	memoryViewAttachInfo->format.channelFormat = VK_CH_FMT_R32G32B32A32;
	memoryViewAttachInfo->format.numericFormat = VK_NUM_FMT_FLOAT;
	vkAttachMemoryViewDescriptors( descriptorSet, 2, 1, memoryViewAttachInfo );

	// X, Y, Z, weight
// 		memoryViewAttachInfo->offset = sizeof( float ) * 4 + sizeof( uint32_t ) + sizeof( float ) * 4;
	memoryViewAttachInfo->offset = sizeof( float ) * 4 + sizeof( float ) * 4 + sizeof( float ) * 4;
	memoryViewAttachInfo->format.channelFormat = VK_CH_FMT_R32G32B32A32;
	memoryViewAttachInfo->format.numericFormat = VK_NUM_FMT_FLOAT;
	vkAttachMemoryViewDescriptors( descriptorSet, 3, 1, memoryViewAttachInfo );
}


uint64_t VulkanInstance::ReferenceImage( Image* image )
{
	// TODO
	return 0;
}


void VulkanInstance::UnreferenceImage( uint64_t ref )
{
	// TODO
}
