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
#include "VulkanObject.h"
#include "File.h"
#include "Debug.h"

// #include <algorithm>


using namespace GE;

extern "C" GE::Object* CreateObject( Vertex* verts, uint32_t nVerts ) {
	return new VulkanObject( verts, nVerts );
}

extern "C" GE::Object* LoadObject( const std::string filename, Instance* instance ) {
	return new VulkanObject( filename, instance );
}


VulkanObject::VulkanObject( Vertex* verts, uint32_t nVerts )
	: Object( verts, nVerts )
{
}


VulkanObject::VulkanObject( const std::string filename, Instance* instance )
	: Object( filename, instance )
{
}


VulkanObject::~VulkanObject()
{
// 	Object::~Object();
}


VK_DESCRIPTOR_SET VulkanObject::descriptorSet( Instance* instance )
{
	if ( mVkRefs.find( instance ) == mVkRefs.end() ){
		AllocateGpu( instance );
	}

	return std::get<0>( mVkRefs.at( instance ) );
}


VK_MEMORY_REF VulkanObject::verticesRef( Instance* instance)
{
	if ( mVkRefs.find( instance ) == mVkRefs.end() ) {
		AllocateGpu( instance );
	}

	return std::get<2>( mVkRefs.at( instance ) );
}


VK_MEMORY_REF VulkanObject::indicesRef( Instance* instance )
{
	if ( mVkRefs.find( instance ) == mVkRefs.end() ) {
		AllocateGpu( instance );
	}

	return std::get<3>( mVkRefs.at( instance ) );
}


void VulkanObject::AllocateGpu( Instance* instance )
{
	VK_DESCRIPTOR_SET descriptorSet = {};
	VK_MEMORY_REF descriptorMemRef = {};
	VK_MEMORY_REF vertexDataMemRef = {};
	VK_MEMORY_REF matrixMemRef = {};
	VK_CMD_BUFFER_CREATE_INFO bufferCreateInfo = { 0, 0 };
	void* bufferPointer = nullptr;

	VK_DESCRIPTOR_SET_CREATE_INFO descriptorCreateInfo = {};
	descriptorCreateInfo.slots = 5;
	vkCreateDescriptorSet( instance->device(), &descriptorCreateInfo, &descriptorSet );
	descriptorMemRef = ((VulkanInstance*)instance)->AllocateObject( descriptorSet );


	vertexDataMemRef = ((VulkanInstance*)instance)->AllocateMappableBuffer( sizeof( Vertex ) * mVerticesCount );
	vkMapMemory( vertexDataMemRef.mem, 0, &bufferPointer );
	if ( bufferPointer ) {
		memcpy( bufferPointer, mVertices, sizeof( Vertex ) * mVerticesCount );
		vkUnmapMemory( vertexDataMemRef.mem );
	} else {
		gDebug() << "Error : vkMapMemory(vertexDataMemRef) returned null pointer\n";
	}


	matrixMemRef = ((VulkanInstance*)instance)->AllocateMappableBuffer( sizeof( float ) * 16 );
	vkMapMemory( matrixMemRef.mem, 0, &bufferPointer );
	if ( bufferPointer ) {
		memcpy( bufferPointer, mMatrix->data(), sizeof( float ) * 16 );
		vkUnmapMemory( matrixMemRef.mem );
	} else {
		gDebug() << "Error : vkMapMemory(matrixMemRef) returned null pointer\n";
	}


	VK_CMD_BUFFER initDataCmdBuffer;
	vkCreateCommandBuffer( instance->device(), &bufferCreateInfo, &initDataCmdBuffer );
	vkBeginCommandBuffer( initDataCmdBuffer, 0 );
		VK_MEMORY_STATE_TRANSITION dataTransition = {};
		dataTransition.mem = vertexDataMemRef.mem;
		dataTransition.oldState = VK_MEMORY_STATE_DATA_TRANSFER;
		dataTransition.newState = VK_MEMORY_STATE_GRAPHICS_SHADER_READ_ONLY;
		dataTransition.offset = 0;
		dataTransition.regionSize = sizeof( Vertex ) * mVerticesCount;
		vkCmdPrepareMemoryRegions( initDataCmdBuffer, 1, &dataTransition );
	vkEndCommandBuffer( initDataCmdBuffer );
	((VulkanInstance*)instance)->QueueSubmit( initDataCmdBuffer, &vertexDataMemRef, 1 );


	vkBeginDescriptorSetUpdate( descriptorSet );
		VK_MEMORY_VIEW_ATTACH_INFO memoryViewAttachInfo = {};
		memoryViewAttachInfo.state = VK_MEMORY_STATE_GRAPHICS_SHADER_READ_ONLY;
		memoryViewAttachInfo.mem = vertexDataMemRef.mem;
		memoryViewAttachInfo.range = sizeof( Vertex ) * mVerticesCount;

		VulkanInstance::UpdateDescriptorSet( descriptorSet, &memoryViewAttachInfo );

		/// HACK / TBD
		// VulkanObject matrix
		memoryViewAttachInfo.state = VK_MEMORY_STATE_GRAPHICS_SHADER_READ_ONLY;
		memoryViewAttachInfo.mem = matrixMemRef.mem;
		memoryViewAttachInfo.stride = 0;
		memoryViewAttachInfo.range = sizeof( float ) * 16;
		memoryViewAttachInfo.offset = 0;
		memoryViewAttachInfo.format.channelFormat = VK_CH_FMT_R32G32B32A32;
		memoryViewAttachInfo.format.numericFormat = VK_NUM_FMT_FLOAT;
		vkAttachMemoryViewDescriptors( descriptorSet, 12, 1, &memoryViewAttachInfo );
		// END HACK / TBD

	vkEndDescriptorSetUpdate( descriptorSet );


	VK_MEMORY_REF indexMemRef = ((VulkanInstance*)instance)->AllocateMappableBuffer( sizeof(uint32_t) * mIndicesCount );
	vkMapMemory( indexMemRef.mem, 0, &bufferPointer );
	if ( bufferPointer ) {
		memcpy( bufferPointer, mIndices, sizeof(uint32_t) * mIndicesCount );
		vkUnmapMemory( indexMemRef.mem );
	} else {
		gDebug() << "Error : vkMapMemory(indexMemRef) returned null pointer\n";
	}

/*	TEST
	vkBeginCommandBuffer( initDataCmdBuffer, 0 );
		VK_MEMORY_STATE_TRANSITION dataTransition = {};
		dataTransition.mem = indexMemRef.mem;
		dataTransition.oldState = VK_MEMORY_STATE_DATA_TRANSFER;
		dataTransition.newState = VK_MEMORY_STATE_GRAPHICS_SHADER_READ_ONLY; // TEST VK_MEMORY_STATE_MULTI_SHADER_READ_ONLY
		dataTransition.offset = 0;
		dataTransition.regionSize = sizeof( decltype(mIndices) ) * mIndicesCount;
		vkCmdPrepareMemoryRegions( initDataCmdBuffer, 1, &dataTransition );
	vkEndCommandBuffer( initDataCmdBuffer );
	instance->QueueSubmit( devid, initDataCmdBuffer, &indexMemRef, 1 );
*/

	std::tuple< VK_DESCRIPTOR_SET, VK_MEMORY_REF, VK_MEMORY_REF, VK_MEMORY_REF, VK_MEMORY_REF > data( descriptorSet, descriptorMemRef, vertexDataMemRef, indexMemRef, matrixMemRef );
	mVkRefs.insert( std::pair< decltype(instance), decltype(data) > ( instance, data ) );
}


void VulkanObject::UpdateVertices( Instance* instance, Vertex* verts, uint32_t offset, uint32_t count )
{
	if ( mVkRefs.find( instance ) == mVkRefs.end() ) {
		return;
	}

	VK_MEMORY_REF vertexDataMemRef = std::get<2>( mVkRefs.at( instance ) );
	void* bufferPointer = nullptr;

	vkMapMemory( vertexDataMemRef.mem, 0, &bufferPointer );
	if ( bufferPointer ) {
		memcpy( (void*)( (uint64_t)bufferPointer + sizeof( Vertex ) * offset ), verts, sizeof( Vertex ) * count );
		vkUnmapMemory( vertexDataMemRef.mem );
	} else {
		gDebug() << "Error : vkMapMemory(vertexDataMemRef) returned null pointer\n";
	}
}


void VulkanObject::UploadMatrix( Instance* instance )
{
	if ( mVkRefs.find( instance ) == mVkRefs.end() ) {
		return;
	}

	VK_MEMORY_REF matrixMemRef = std::get<4>( mVkRefs.at( instance ) );
	void* bufferPointer = nullptr;

	vkMapMemory( matrixMemRef.mem, 0, &bufferPointer );
	if ( bufferPointer ) {
		memcpy( bufferPointer, mMatrix->data(), sizeof( float ) * 16 );
		vkUnmapMemory( matrixMemRef.mem );
	} else {
		gDebug() << "Error : vkMapMemory(matrixMemRef) returned null pointer\n";
	}
}


void VulkanObject::setTexture( Instance* Instance, int unit, Image* texture )
{
}
