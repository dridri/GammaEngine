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

#include "VulkanInstance.h"
#include "VulkanObject.h"
#include "File.h"
#include "Debug.h"
#include "Image.h"

// #include <algorithm>


using namespace GE;

extern "C" GE::Object* CreateObject( Vertex* verts, uint32_t nVerts, uint32_t* indices, uint32_t nIndices ) {
	return new VulkanObject( verts, nVerts, indices, nIndices );
}

extern "C" GE::Object* LoadObject( const std::string filename, Instance* instance ) {
	return new VulkanObject( filename, instance );
}


VulkanObject::VulkanObject()
	: Object( 0, 0 )
{
}


VulkanObject::VulkanObject( Vertex* verts, uint32_t nVerts, uint32_t* indices, uint32_t nIndices )
	: Object( verts, nVerts, indices, nIndices )
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

/*
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
*/

/*
VkCommandBuffer& VulkanObject::commandBuffer( VulkanInstance* instance )
{
	if ( mCommandBuffers.find( instance ) == mCommandBuffers.end() ){
		AllocateGpu( instance );
	}
	return mCommandBuffers[instance];
}
*/
/*
VkBuffer& VulkanObject::vertexBuffer( VulkanInstance* instance )
{
	if ( mVertexBuffers.find( instance ) == mVertexBuffers.end() ) {
		AllocateGpu( instance );
	}
	return mVertexBuffers[instance].second;
}


VkBuffer& VulkanObject::indicesBuffer( VulkanInstance* instance )
{
	if ( mIndicesCount == 0 ) {
		static VkBuffer _null_buffer = VK_NULL_HANDLE;
		return _null_buffer;
	}
	if ( mIndicesBuffers.find( instance ) == mIndicesBuffers.end() ) {
		AllocateGpu( instance );
	}
	return mIndicesBuffers[instance].second;
}
*/
VkBuffer& VulkanObject::matricesBuffer( VulkanInstance* instance )
{
	if ( mMatricesBuffers.find( instance ) == mMatricesBuffers.end() ) {
		AllocateGpu( instance );
	}
	return mMatricesBuffers[instance].second;
}


const std::vector<VulkanTexture*> * VulkanObject::textures( VulkanInstance* instance )
{
	if ( mTextures.find( instance ) != mTextures.end() ) {
		return &mTextures[instance];
	}
	return nullptr;
}


void VulkanObject::AllocateGpu( VulkanInstance* instance )
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
/*
	instance->CreateBuffer( mVerticesCount * Vertex::vertexDefinition().size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &stagingBuffer, &stagingBufferMemory );
	mVertexBuffers[instance] = std::make_pair( stagingBufferMemory, stagingBuffer );
	UpdateVertices( instance, mVertices, 0, mVerticesCount );

	if ( mIndicesCount > 0 ) {
		instance->CreateBuffer( mIndicesCount * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &stagingBuffer, &stagingBufferMemory );
		mIndicesBuffers[instance] = std::make_pair( stagingBufferMemory, stagingBuffer );
		UpdateIndices( instance, mIndices, 0, mIndicesCount );
	}
*/
	instance->CreateBuffer( mMatrices.size() * sizeof(float)*16, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory );
	mMatricesBuffers[instance] = std::make_pair( stagingBufferMemory, stagingBuffer );
	UploadMatrix( instance );
}


void VulkanObject::UpdateVertices( Instance* instance, VertexBase* verts, uint32_t offset, uint32_t count )
{
/*
	VulkanInstance* inst = static_cast<VulkanInstance*>(instance);
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	uint32_t bufferOffset = offset * Vertex::vertexDefinition().size();
	uint32_t bufferSize = count * Vertex::vertexDefinition().size();

	inst->CreateBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory );
	void* data;
	vkMapMemory( inst->device(), stagingBufferMemory, 0, bufferSize, 0, &data );
	memcpy( data, verts, (size_t)bufferSize );
	vkUnmapMemory( inst->device(), stagingBufferMemory );

	inst->CopyBuffer( mVertexBuffers[inst].second, stagingBuffer, bufferOffset, bufferSize );

	vkDestroyBuffer( inst->device(), stagingBuffer, nullptr );
	vkFreeMemory( inst->device(), stagingBufferMemory, nullptr );
*/
}


void VulkanObject::UpdateIndices( Instance* instance, uint32_t* indices, uint32_t offset, uint32_t count )
{
/*
	VulkanInstance* inst = static_cast<VulkanInstance*>(instance);
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	uint32_t bufferOffset = offset * sizeof(uint32_t);
	uint32_t bufferSize = count * sizeof(uint32_t);

	inst->CreateBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory );
	void* data;
	vkMapMemory( inst->device(), stagingBufferMemory, 0, bufferSize, 0, &data );
	memcpy( data, indices, (size_t)bufferSize );
	vkUnmapMemory( inst->device(), stagingBufferMemory );
	inst->CopyBuffer( mIndicesBuffers[inst].second, stagingBuffer, bufferOffset, bufferSize );

	vkDestroyBuffer( inst->device(), stagingBuffer, nullptr );
	vkFreeMemory( inst->device(), stagingBufferMemory, nullptr );
*/
}


void VulkanObject::ReuploadVertices( Renderer* renderer, uint32_t offset, uint32_t count )
{
}


void VulkanObject::UploadMatrix( Instance* instance )
{
	VulkanInstance* inst = static_cast<VulkanInstance*>(instance);
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	uint32_t bufferSize = mMatrices.size() * sizeof(float) * 16;

	stagingBufferMemory = mMatricesBuffers[inst].first;

// 	inst->CreateBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory );
	float* data;
	vkMapMemory( inst->device(), stagingBufferMemory, 0, bufferSize, 0, (void**)&data );
	for ( Matrix* m : mMatrices ) {
		memcpy( data, m->data(), sizeof(float) * 16 );
		data += 16;
	}
	vkUnmapMemory( inst->device(), stagingBufferMemory );

// 	inst->CopyBuffer( mMatricesBuffers[inst].second, stagingBuffer, 0, bufferSize );
// 	vkDestroyBuffer( inst->device(), stagingBuffer, nullptr );
// 	vkFreeMemory( inst->device(), stagingBufferMemory, nullptr );
}


void VulkanObject::setTexture( Instance* instance_, int unit, Image* texture )
{
	VulkanInstance* instance = static_cast<VulkanInstance*>(instance_);

	std::vector< VulkanTexture* >* textures = nullptr;
	if ( mTextures.find( instance ) == mTextures.end() ) {
		std::vector< VulkanTexture* > empty;
		mTextures.insert( std::make_pair( instance, empty ) );
	}
	textures = &mTextures[ instance ];

	if ( (int)textures->size() <= unit ) {
		textures->resize( unit + 1, nullptr );
	}

	(*textures)[unit] = reinterpret_cast<VulkanTexture*>( texture->serverReference( instance ) );
}
