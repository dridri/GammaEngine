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

#include <fstream>
#include <vector>

#include "VulkanInstance.h"
#include "VulkanRenderer.h"
#include "VulkanFramebuffer.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"

extern "C" GE::Renderer* CreateRenderer( GE::Instance* instance ) {
	return new VulkanRenderer( instance );
}


VulkanRenderer::VulkanRenderer( Instance* instance )
	: mReady( false )
	, mInstance( static_cast< VulkanInstance* >( instance ? instance : Instance::baseInstance() ) )
	, mMatrixObjects( 0 )
	, mVertexDefinition( Vertex::vertexDefinition() )
	, mCommandBuffer( 0 )
	, mPipelineLayout( 0 )
	, mPipeline( 0 )
// 	, mRenderMode( VK_TOPOLOGY_TRIANGLE_LIST )
{
/*
	VK_CMD_BUFFER_CREATE_INFO info;

	info.queueType = 0;
	info.flags = VK_CMD_BUFFER_OPTIMIZE_DESCRIPTOR_SET_SWITCH | VK_CMD_BUFFER_OPTIMIZE_GPU_SMALL_BATCH;

	vkCreateCommandBuffer( mInstance->device(), &info, &mCmdBuffer );
*/
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


void VulkanRenderer::setVertexDefinition( const VertexDefinition& vertexDefinition )
{
	mVertexDefinition = vertexDefinition;
}


void VulkanRenderer::setRenderMode( const RenderMode& mode )
{
}


void VulkanRenderer::setDepthTestEnabled( bool en )
{
}


void VulkanRenderer::setBlendingEnabled( bool en )
{
}


void VulkanRenderer::setBlendingMode( BlendingMode source, BlendingMode dest )
{
}


int VulkanRenderer::LoadVertexShader( const void* data, size_t size )
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = static_cast<const uint32_t*>(data);

	if ( vkCreateShaderModule( mInstance->device(), &createInfo, nullptr, &mVertexShader ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create shader module!");
	}

	mReady = false;
	return 0;
}


int VulkanRenderer::LoadGeometryShader( const void* data, size_t size )
{
	return 0;
}


int VulkanRenderer::LoadFragmentShader( const void* data, size_t size )
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = static_cast<const uint32_t*>(data);

	if ( vkCreateShaderModule( mInstance->device(), &createInfo, nullptr, &mFragmentShader ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create shader module!");
	}

	mReady = false;
	return 0;
}


int VulkanRenderer::LoadVertexShader( const std::string& file )
{
	size_t size = 0;
	void* data = loadShader( file, &size );
	return LoadVertexShader( data, size );
}


int VulkanRenderer::LoadGeometryShader( const std::string& file )
{
	return 0;
}


int VulkanRenderer::LoadFragmentShader( const std::string& file )
{
	size_t size = 0;
	void* data = loadShader( file, &size );
	return LoadFragmentShader( data, size );
}


void VulkanRenderer::createPipeline()
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = mVertexShader;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = mFragmentShader;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = mVertexDefinition.size();
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attributeDescriptions[4] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = mVertexDefinition.attributes()[0].offset,
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = mVertexDefinition.attributes()[1].offset,
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = mVertexDefinition.attributes()[2].offset,
		},
		{
			.location = 3,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = mVertexDefinition.attributes()[3].offset,
		},
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = 4;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
/*
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = 1280.0f;
	viewport.height = 720.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = {1280, 720};
*/
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
// 	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
// 	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;


	std::vector<VkDescriptorSetLayoutBinding> uniforms;
	// Matrices
	uniforms.push_back( { .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_VERTEX_BIT } );
	mInstance->CreateBuffer( 2*sizeof(float)*16, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &mUniformsBuffer.second, &mUniformsBuffer.first );

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = uniforms.size();
	layoutInfo.pBindings = uniforms.data();
	if ( vkCreateDescriptorSetLayout( mInstance->device(), &layoutInfo, nullptr, &mDescriptorSetLayout ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

/*
	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.size = sizeof(mPushConstants);
	pushConstantRange.offset = 0;
*/
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
// 	pipelineLayoutInfo.pushConstantRangeCount = 1;
// 	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;

	if ( mPipelineLayout ) {
		vkDestroyPipelineLayout( mInstance->device(), mPipelineLayout, nullptr );
	}
	if ( vkCreatePipelineLayout( mInstance->device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	std::vector< VkDynamicState > dynamicsStates;
	dynamicsStates.push_back( VK_DYNAMIC_STATE_VIEWPORT );
	dynamicsStates.push_back( VK_DYNAMIC_STATE_SCISSOR );
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateInfo = {};
	pipelineDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateInfo.dynamicStateCount = dynamicsStates.size();
	pipelineDynamicStateInfo.pDynamicStates = dynamicsStates.data();

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &pipelineDynamicStateInfo;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mInstance->renderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if ( mPipeline ) {
		vkDestroyPipeline( mInstance->device(), mPipeline, nullptr );
	}
	if ( vkCreateGraphicsPipelines( mInstance->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}


	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1;
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	if ( vkCreateDescriptorPool( mInstance->device(), &poolInfo, nullptr, &mDescriptorPool ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &mDescriptorSetLayout;
	if ( vkAllocateDescriptorSets( mInstance->device(), &allocInfo, &mUniformsDescriptorSet ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = mUniformsBuffer.second;
	bufferInfo.offset = 0;
	bufferInfo.range = 2*sizeof(float)*16;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = mUniformsDescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets( mInstance->device(), 1, &descriptorWrite, 0, nullptr );
}


void VulkanRenderer::PopulateCommandBuffer( VkCommandBuffer commandBuffer )
{
	for ( VulkanObject* object : mObjects ) {
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers( commandBuffer, 0, 1, &object->vertexBuffer(mInstance), offsets );
		if ( object->indicesCount() > 0 ) {
			vkCmdBindIndexBuffer( commandBuffer, object->indicesBuffer(mInstance), 0, VK_INDEX_TYPE_UINT32 );
			vkCmdDrawIndexed( commandBuffer, object->indicesCount(), object->instancesCount(), 0, 0, 0 );
		} else {
			vkCmdDraw( commandBuffer, object->verticesCount(), object->instancesCount(), 0, 0 );
		}
	}
}


void VulkanRenderer::AddObject( Object* obj )
{
	mObjects.emplace_back( (VulkanObject*)obj );
}


void VulkanRenderer::AddLight( Light* light )
{
	mLights.emplace_back( light );
}


void VulkanRenderer::VertexPoolAppend( VertexBase** pVertices, uint32_t& pVerticesPoolSize, uint32_t& pVerticesCount, VertexBase* append, uint32_t count )
{
	if ( pVerticesCount + count > pVerticesPoolSize ) {
		pVerticesPoolSize += ( ( count + 8192 ) / 8192 * 8192 );
		VertexBase* vertices = (VertexBase*)mInstance->Malloc( mVertexDefinition.size() * pVerticesPoolSize, false );
		if ( *pVertices ) {
			memcpy( vertices, *pVertices, mVertexDefinition.size() * pVerticesCount );
			mInstance->Free( *pVertices );
		}
		*pVertices = vertices;
	}

	memcpy( &((uint8_t*)*pVertices)[ mVertexDefinition.size() * pVerticesCount ], append, mVertexDefinition.size() * count );

	pVerticesCount += count;
}


void VulkanRenderer::Compute()
{
	if ( !mReady ) {
		createPipeline();
	}
	mReady = true;



/*
	VertexBase* vertices = nullptr;
	std::vector< uint32_t > indices;
	std::vector< uint32_t > IDs;
// 	std::vector< uint32_t > textureBases;
// 	std::vector< uint32_t > textureCount;
	uint32_t verticesPoolSize = 0;
	uint32_t verticesCount = 0;
	uint32_t indicesCount = 0;
	uint32_t total_instances = 0;

	mTotalObjectsInstances = 0;
	if ( mMatrixObjects ) {
		mInstance->Free( mMatrixObjects );
	}
	for ( size_t i = 0; i < mObjects.size(); i++ ) {
		mTotalObjectsInstances += mObjects[i]->instancesCount();
	}
	mMatrixObjects = (float*)mInstance->Malloc( sizeof(float) * 16 * mTotalObjectsInstances );

	int curr = 0;
	uint32_t texID = 0;

	for ( size_t i = 0; i < mObjects.size(); i++ ) {
		IDs.emplace_back( (uint32_t)i );
		for ( int j = 0; j < mObjects[i]->instancesCount(); j++ ) {
			mObjects[i]->matrix( j )->setDataPointer( &mMatrixObjects[ 16 * ( total_instances + j )] );
		}

		indices.insert( indices.end(), mObjects[i]->indices(), &mObjects[i]->indices()[mObjects[i]->indicesCount()] );
		VertexPoolAppend( &vertices, verticesPoolSize, verticesCount, mObjects[i]->vertices(), mObjects[i]->verticesCount() );
		indicesCount += mObjects[i]->indicesCount();

		const std::vector< std::pair< Image*, uint32_t > >* textures = ((VulkanObject*)mObjects[i])->textures( mInstance );
		if ( textures ) {
			for ( int k = 0; k < mObjects[i]->instancesCount(); k++ ) {
				textureBases.emplace_back( texID );
				textureCount.emplace_back( textures->size() );
			}
			for ( uint32_t k = 0; k < textures->size(); k++ ) {
				mTexturesNames.emplace_back( textures->at(k).second );
				texID++;
			}
		} else {
			for ( int k = 0; k < mObjects[i]->instancesCount(); k++ ) {
				textureBases.emplace_back( 0x00000000 );
				textureCount.emplace_back( 0x00000000 );
			}
		}

// 		std::cout << "textureBases[" << i << "] = " << std::hex << textureBases.back() << "\n";
		total_instances += mObjects[i]->instancesCount();
	}
	gDebug() << "mObjects.size() : " << mObjects.size() << "\n";
	gDebug() << "vertices.size() : " << verticesCount << "\n";
	gDebug() << "indices.size() : " << indices.size() << "\n";
// 	gDebug() << "textureBases.size() : " << textureBases.size() << "\n";
// 	gDebug() << "textureCount.size() : " << textureCount.size() << "\n";
*/

/*
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = mVertexDefinition.size() * verticesCount;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if ( vkCreateBuffer( mInstance->device(), &bufferInfo, nullptr, &mVertexBuffer ) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}
*/
/*
	if ( mCommandBuffer == 0 ) {
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mInstance->commandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = 1;
		if ( vkAllocateCommandBuffers( mInstance->device(), &allocInfo, &mCommandBuffer ) != VK_SUCCESS ) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	} else {
		vkResetCommandBuffer( mCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
	}

	const VkCommandBufferInheritanceInfo inherit = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		.renderPass = mInstance->renderPass(),
	};
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	beginInfo.pInheritanceInfo = &inherit;

	if ( vkBeginCommandBuffer( mCommandBuffer, &beginInfo ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	vkCmdBindPipeline( mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline );
	vkCmdBindDescriptorSets( mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mUniformsDescriptorSet, 0, nullptr );
	PopulateCommandBuffer( mCommandBuffer );

	if ( vkEndCommandBuffer( mCommandBuffer ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to record command buffer!");
	}
*/
}


void VulkanRenderer::Draw()
{
	if ( !mReady ) {
		Compute();
	}

	for ( decltype(mObjects)::iterator it = mObjects.begin(); it != mObjects.end(); ++it ) {
		(*it)->UploadMatrix( mInstance );
	}

	VulkanFramebuffer* currentFramebuffer = mInstance->boundFramebuffer(Thread::currentThread());
	bool exists = ( mRenderCommandBuffers.count( currentFramebuffer ) > 0 );
	bool changed = ( exists and ( currentFramebuffer->hash() != mFramebuffersHashes[currentFramebuffer] ) );

	if ( exists == false or changed ) {
		gDebug() << "Creating new fb commands";
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mInstance->commandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if ( changed ) {
			commandBuffer = mRenderCommandBuffers[currentFramebuffer];
			vkResetCommandBuffer( commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
		} else {
			if ( vkAllocateCommandBuffers( mInstance->device(), &allocInfo, &commandBuffer ) != VK_SUCCESS ) {
				throw std::runtime_error("failed to allocate command buffers!");
			}
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mInstance->renderPass();
		renderPassInfo.framebuffer = currentFramebuffer->framebuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = VkExtent2D{ (uint32_t)currentFramebuffer->viewport().width, (uint32_t)currentFramebuffer->viewport().height };

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		if ( vkBeginCommandBuffer( commandBuffer, &beginInfo ) != VK_SUCCESS ) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
			vkCmdSetViewport( commandBuffer, 0, 1, &currentFramebuffer->viewport() );
			vkCmdSetScissor( commandBuffer, 0, 1, &currentFramebuffer->scissors() );
			vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline );
			vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mUniformsDescriptorSet, 0, nullptr );
			PopulateCommandBuffer( commandBuffer );
// 			vkCmdExecuteCommands( commandBuffer, 1, &mCommandBuffer );
		vkCmdEndRenderPass( commandBuffer );
		if ( vkEndCommandBuffer( commandBuffer ) != VK_SUCCESS ) {
			throw std::runtime_error("failed to record command buffer!");
		}

		mRenderCommandBuffers[currentFramebuffer] = commandBuffer;
		mFramebuffersHashes[currentFramebuffer] = currentFramebuffer->hash();
	}

	void* data;
	vkMapMemory( mInstance->device(), mUniformsBuffer.first, 0, 2*sizeof(float)*16, 0, &data );
	memcpy( &((float*)data)[0], mMatrixProjection->data(), sizeof(float) * 16 );
	memcpy( &((float*)data)[16], mMatrixView->data(), sizeof(float) * 16 );
	vkUnmapMemory( mInstance->device(), mUniformsBuffer.first );

	VkSubmitInfo submitInfo = {};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &currentFramebuffer->imageAvailableSemaphores();
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &currentFramebuffer->imageAvailableSemaphores();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mRenderCommandBuffers[currentFramebuffer];
	currentFramebuffer->waitFence();
	if ( vkQueueSubmit( mInstance->presentationQueue(), 1, &submitInfo, currentFramebuffer->fence() ) != VK_SUCCESS ) {
		std::cerr << "failed to submit draw command buffer" << std::endl;
		exit(1);
	}
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

	uint8_t* data = (uint8_t*)mInstance->Malloc( size + 1 );

	file->Read( data, size );
	data[size] = 0;
	delete file;

	if ( sz ) {
		*sz = size;
	}
	return data;
}
