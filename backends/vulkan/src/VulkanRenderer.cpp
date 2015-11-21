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

#include <fstream>
#include <vector>

#include "VulkanInstance.h"
#include "VulkanRenderer.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"

extern "C" GE::Renderer* CreateRenderer( GE::Instance* instance ) {
	return new VulkanRenderer( instance );
}


VulkanRenderer::VulkanRenderer( Instance* instance )
	: mReady( false )
	, mInstance( instance ? instance : Instance::baseInstance() )
	, mPipeline( 0 )
	, mRenderMode( VK_TOPOLOGY_TRIANGLE_LIST )
{
	VK_CMD_BUFFER_CREATE_INFO info;

	info.queueType = 0;
	info.flags = VK_CMD_BUFFER_OPTIMIZE_DESCRIPTOR_SET_SWITCH | VK_CMD_BUFFER_OPTIMIZE_GPU_SMALL_BATCH;

	vkCreateCommandBuffer( mInstance->device(), &info, &mCmdBuffer );

	mMatrixProjection = new Matrix();
	mMatrixProjection->Perspective( 60.0f, 16.0f / 9.0f, 0.01f, 1000.0f );
	mMatrixView = new Matrix();
	mMatrixView->Identity();
}


VulkanRenderer::~VulkanRenderer()
{
}


Matrix* VulkanRenderer::projectionMatrix()
{
	return mMatrixProjection;
}


int VulkanRenderer::LoadVertexShader( const void* data, size_t size )
{
	VK_SHADER_CREATE_INFO vsInfo = { 0 };
	vsInfo.pCode = data;
	vsInfo.codeSize = size;

	vkCreateShader( mInstance->device(), &vsInfo, &mVertexShader );

	mReady = false;
	return 0;
}


int VulkanRenderer::LoadFragmentShader( const void* data, size_t size )
{
	VK_SHADER_CREATE_INFO vsInfo = { 0 };
	vsInfo.pCode = data;
	vsInfo.codeSize = size;

	vkCreateShader( mInstance->device(), &vsInfo, &mFragmentShader );

	mReady = false;
	return 0;
}


int VulkanRenderer::LoadVertexShader( const std::string& file )
{
	size_t size = 0;
	void* data = loadShader( file, &size );
	return LoadVertexShader( data, size );
}


int VulkanRenderer::LoadFragmentShader( const std::string& file )
{
	size_t size = 0;
	void* data = loadShader( file, &size );
	return LoadFragmentShader( data, size );
}


void VulkanRenderer::setRenderMode( int mode )
{
	mRenderMode = mode;
}


void VulkanRenderer::createPipeline()
{
	if ( mPipeline ) {
		// TODO : Free existing pipeline
	}

	uint32_t nvs = 0;
	VK_DESCRIPTOR_SLOT_INFO vsDescriptorSlots[16];
	VK_DESCRIPTOR_SLOT_INFO psDescriptorSlots[16];
	VK_LINK_CONST_BUFFER vsLinkConstBuffers[16];
	VK_GRAPHICS_PIPELINE_CREATE_INFO pipelineCreateInfo = {};

	vsDescriptorSlots[nvs  ].slotObjectType = VK_SLOT_SHADER_RESOURCE;
	vsDescriptorSlots[nvs++].shaderEntityIndex = 0;
	vsDescriptorSlots[nvs  ].slotObjectType = VK_SLOT_SHADER_RESOURCE;
	vsDescriptorSlots[nvs++].shaderEntityIndex = 1;
	vsDescriptorSlots[nvs  ].slotObjectType = VK_SLOT_SHADER_RESOURCE;
	vsDescriptorSlots[nvs++].shaderEntityIndex = 2;
	vsDescriptorSlots[nvs  ].slotObjectType = VK_SLOT_SHADER_RESOURCE;
	vsDescriptorSlots[nvs++].shaderEntityIndex = 3;
	vsDescriptorSlots[nvs  ].slotObjectType = VK_SLOT_SHADER_RESOURCE;
	vsDescriptorSlots[nvs++].shaderEntityIndex = 10;
	vsDescriptorSlots[nvs  ].slotObjectType = VK_SLOT_SHADER_RESOURCE;
	vsDescriptorSlots[nvs++].shaderEntityIndex = 11;
	vsDescriptorSlots[nvs  ].slotObjectType = VK_SLOT_SHADER_RESOURCE;
	vsDescriptorSlots[nvs++].shaderEntityIndex = 12;

	vsLinkConstBuffers[0].bufferId = 10;
	vsLinkConstBuffers[0].bufferSize = sizeof(float) * 16;
	vsLinkConstBuffers[0].pBufferData = mMatrixProjection->data();
	vsLinkConstBuffers[1].bufferId = 11;
	vsLinkConstBuffers[1].bufferSize = sizeof(float) * 16;
	vsLinkConstBuffers[1].pBufferData = mMatrixView->data();

	psDescriptorSlots[0].slotObjectType = VK_SLOT_UNUSED;
	psDescriptorSlots[1].slotObjectType = VK_SLOT_UNUSED;

	pipelineCreateInfo.vs.shader = mVertexShader;
	pipelineCreateInfo.vs.dynamicMemoryViewMapping.slotObjectType = VK_SLOT_UNUSED;
	pipelineCreateInfo.vs.descriptorSetMapping[0].descriptorCount = nvs;
	pipelineCreateInfo.vs.descriptorSetMapping[0].pDescriptorInfo = vsDescriptorSlots;
	pipelineCreateInfo.vs.linkConstBufferCount = 2;
	pipelineCreateInfo.vs.pLinkConstBufferInfo = vsLinkConstBuffers;

	pipelineCreateInfo.ps.shader = mFragmentShader;
	pipelineCreateInfo.ps.dynamicMemoryViewMapping.slotObjectType = VK_SLOT_UNUSED;
	pipelineCreateInfo.ps.descriptorSetMapping[0].descriptorCount = 2;
	pipelineCreateInfo.ps.descriptorSetMapping[0].pDescriptorInfo = psDescriptorSlots;

	pipelineCreateInfo.iaState.topology = mRenderMode;
	pipelineCreateInfo.iaState.disableVertexReuse = VK_FALSE;
	pipelineCreateInfo.rsState.depthClipEnable = VK_FALSE;

	pipelineCreateInfo.cbState.logicOp = VK_LOGIC_OP_COPY;
	pipelineCreateInfo.cbState.target[0].blendEnable = VK_TRUE;
	pipelineCreateInfo.cbState.target[0].channelWriteMask = 0xF; // RGBA bits
	pipelineCreateInfo.cbState.target[0].format.channelFormat = VK_CH_FMT_R8G8B8A8;
	pipelineCreateInfo.cbState.target[0].format.numericFormat = VK_NUM_FMT_UNORM;
	pipelineCreateInfo.dbState.format.channelFormat = VK_CH_FMT_R4G4B4A4;
	pipelineCreateInfo.dbState.format.numericFormat = VK_NUM_FMT_UNDEFINED;

	vkCreateGraphicsPipeline( mInstance->device(), &pipelineCreateInfo, &mPipeline );
	mPipelineRef = ((VulkanInstance*)mInstance)->AllocateObject( mPipeline );

	mReady = true;
}


void VulkanRenderer::AddObject( Object* obj )
{
	mObjects.emplace_back( (VulkanObject*)obj );
}


void VulkanRenderer::AddLight( Light* light )
{
	mLights.emplace_back( light );
}


void VulkanRenderer::Compute()
{
	if ( !mReady ) {
		createPipeline();
	}

	vkBeginCommandBuffer( mCmdBuffer, 0 );

/*	TODO TODO TODO
	vkCmdBindStateObject( mCmdBuffer, VK_STATE_BIND_MSAA, msaaState );
	vkCmdBindStateObject( mCmdBuffer, VK_STATE_BIND_VIEWPORT, viewportState );
	vkCmdBindStateObject( mCmdBuffer, VK_STATE_BIND_COLOR_BLEND, colorBlendState );
	vkCmdBindStateObject( mCmdBuffer, VK_STATE_BIND_DEPTH_STENCIL, depthStencilState );
	vkCmdBindStateObject( mCmdBuffer, VK_STATE_BIND_RASTER, rasterState );
*/
	vkCmdBindPipeline( mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline );

	for ( decltype(mObjects)::iterator it = mObjects.begin(); it != mObjects.end(); ++it ) {
		vkCmdBindDescriptorSet( mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, (*it)->descriptorSet( mInstance ), 0 );
		vkCmdBindVertexBuffer( mCmdBuffer, (*it)->verticesRef( mInstance ).mem, 0, 0 ); // TESTING
		if ( (*it)->indicesCount() != 0 ) {
			vkCmdBindIndexData( mCmdBuffer, (*it)->indicesRef( mInstance ).mem, 0, VK_INDEX_32 );
			vkCmdDrawIndexed( mCmdBuffer, 0, (*it)->indicesCount(), 0, 0, 1 );
		} else {
			vkCmdDraw( mCmdBuffer, 0, (*it)->verticesCount(), 0, 1 );
		}
	}

	vkEndCommandBuffer( mCmdBuffer );
}


void VulkanRenderer::Draw()
{
	if ( !mReady ) {
		Compute();
	}

	for ( decltype(mObjects)::iterator it = mObjects.begin(); it != mObjects.end(); ++it ) {
		(*it)->UploadMatrix( mInstance );
	}

	((VulkanInstance*)mInstance)->QueueSubmit( mCmdBuffer, 0, 0 );
}


void VulkanRenderer::Look( Camera* cam )
{
	memcpy( mMatrixView->data(), cam->data(), sizeof( float ) * 16 );
	// TODO / TBD : upload matrix to shader
}


uintptr_t VulkanRenderer::attributeID( const std::string& name )
{
	return 0;
}


uintptr_t VulkanRenderer::uniformID( const std::string& name )
{
	return 0;
}


void VulkanRenderer::uniformUpload( const uintptr_t id, const float f )
{
}


void VulkanRenderer::uniformUpload( const uintptr_t id, const Vector2f& v )
{
}


void VulkanRenderer::uniformUpload( const uintptr_t id, const Vector3f& v )
{
}


void VulkanRenderer::uniformUpload( const uintptr_t id, const Vector4f& v )
{
}


void VulkanRenderer::uniformUpload( const uintptr_t id, const Matrix& v )
{
}


uint8_t* VulkanRenderer::loadShader( const std::string& filename, size_t* sz )
{
	File* file = new File( filename, File::READ );

	size_t size = file->Seek( 0, File::END );
	file->Rewind();

	uint8_t* data = (uint8_t*)mInstance->Malloc( size );

	file->Read( data, size );
	delete file;

	*sz = size;
	return data;
}
