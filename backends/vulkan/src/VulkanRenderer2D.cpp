/*
 * The GammaEngine Library 2.0 is a multiplatform -based game engine
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
#include "Instance.h"
#include "VulkanObject.h"
#include "VulkanRenderer2D.h"
#include "VulkanWindow.h"
#include "Window.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"
#include "Image.h"
#include "Font.h"
#include "VulkanRenderPass.h"

#define GLSL(version, shader)  "#version " #version "\n" #shader

extern "C" GE::Renderer2D* CreateRenderer2D( GE::Instance* instance, uint32_t width, uint32_t height ) {
	return new VulkanRenderer2D( instance, width, height );
}


VulkanRenderer2D::VulkanRenderer2D( Instance* instance, uint32_t width, uint32_t height )
	: Renderer2D()
	, VulkanRenderer( instance )
	, m2DReady( false )
	, mWidth( width )
	, mHeight( height )
	, mDescriptorsChanged( false )
{
	mDepthTest = false;
	mBlending = true;

	mMatrixProjection->Orthogonal( 0.0, mWidth, mHeight, 0.0, -2049.0, 2049.0 );
	mMatrixView->Identity();

	mTextureWhite = new Image( 1, 1, 0xFFFFFFFF, instance );

	// TODO : load binary default shader then compute
// 	Compute();


	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if ( vkCreateFence( mInstance->device(), &fenceInfo, nullptr, &mSingleDrawsFence ) != VK_SUCCESS ) {
		throw std::runtime_error("failed to create fence object for a frame!");
	}
}


VulkanRenderer2D::~VulkanRenderer2D()
{
}


void VulkanRenderer2D::setDepthTestEnabled( bool en )
{
	VulkanRenderer::setDepthTestEnabled( en );
}


void VulkanRenderer2D::setBlendingEnabled( bool en )
{
	VulkanRenderer::setBlendingEnabled( en );
}


Matrix* VulkanRenderer2D::projectionMatrix()
{
	return mMatrixProjection;
}


Matrix* VulkanRenderer2D::viewMatrix()
{
	return mMatrixView;
}


int VulkanRenderer2D::LoadVertexShader( const void* data, size_t size )
{
	m2DReady = false;
	return VulkanRenderer::LoadVertexShader( data, size );
}


int VulkanRenderer2D::LoadVertexShader( const std::string& file )
{
	m2DReady = false;
	return VulkanRenderer::LoadVertexShader( file );
}


int VulkanRenderer2D::LoadFragmentShader( const void* data, size_t size )
{
	m2DReady = false;
	return VulkanRenderer::LoadFragmentShader( data, size );
}


int VulkanRenderer2D::LoadFragmentShader( const std::string& file )
{
	m2DReady = false;
	return VulkanRenderer::LoadFragmentShader( file );
}


void VulkanRenderer2D::Compute()
{
	createPipeline( 512, true );

	mVertexBuffer.size = 128 * mVertexDefinition.size();
	mInstance->CreateBuffer( mVertexBuffer.size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mVertexBuffer.device.buffer, &mVertexBuffer.device.memory );
	mInstance->CreateBuffer( mVertexBuffer.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mVertexBuffer.host.buffer, &mVertexBuffer.host.memory );

	void* data;
	vkMapMemory( mInstance->device(), mUniformsBuffer.host.memory, 0, mUniformsBuffer.size, 0, &data );
	memcpy( &((float*)data)[0], mMatrixProjection->data(), sizeof(float) * 16 );
	memcpy( &((float*)data)[16], mMatrixView->data(), sizeof(float) * 16 );
	vkUnmapMemory( mInstance->device(), mUniformsBuffer.host.memory );
	mInstance->CopyBuffer( mUniformsBuffer.device.buffer, mUniformsBuffer.host.buffer, 0, mUniformsBuffer.size );

	mIndirectBuffer.size = sizeof(VkDrawIndirectCommand);
	mInstance->CreateBuffer( mIndirectBuffer.size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mIndirectBuffer.device.buffer, &mIndirectBuffer.device.memory );
	mInstance->CreateBuffer( mIndirectBuffer.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &mIndirectBuffer.host.buffer, &mIndirectBuffer.host.memory );

	m2DReady = true;
}


void VulkanRenderer2D::Prerender( GE::Image* image )
{
	if ( !m2DReady ) {
		Compute();
	}
	if ( mAssociatedWindow != nullptr && ( mAssociatedWindow->width() != mWidth || mAssociatedWindow->height() != mHeight ) ) {
		mWidth = mAssociatedWindow->width();
		mHeight = mAssociatedWindow->height();
		mMatrixProjection->Orthogonal( 0.0, mWidth, mHeight, 0.0, -2049.0, 2049.0 );
	}

	vkWaitForFences( mInstance->device(), 1, &mSingleDrawsFence, VK_TRUE, UINT64_MAX );
	vkResetFences( mInstance->device(), 1, &mSingleDrawsFence );

	VulkanTexture* tex = reinterpret_cast<VulkanTexture*>( image->serverReference( mInstance ) );
	if ( mImageInfos.find( image ) == mImageInfos.end() ) {
		VkDescriptorImageInfo imageInfo = {
			.sampler = tex->sampler(),
			.imageView = tex->imageView(),
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};
		mImageInfos[image] = std::make_pair( mImageInfos.size(), imageInfo );
		std::vector<VkDescriptorImageInfo> imageInfos;
		for( auto i : mImageInfos ) {
			imageInfos.push_back( i.second.second );
		}
		for ( uint32_t i = imageInfos.size(); i < 512; i++ ) {
			imageInfos.push_back( { mEmptyTexture->sampler(), mEmptyTexture->imageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL } );
		}
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = mDescriptorSet;
		descriptorWrite.dstBinding = 1;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = imageInfos.size();
		descriptorWrite.pImageInfo = imageInfos.data();
		vkUpdateDescriptorSets( mInstance->device(), 1, &descriptorWrite, 0, nullptr );
		mDescriptorsChanged = true;
	}
}


void VulkanRenderer2D::Render( GE::Image* image, int mode, int start, int n, const Matrix& matrix )
{
	VkDeviceSize offsets[] = { 0 };

	VulkanFramebuffer* currentFramebuffer = mInstance->boundFramebuffer(Thread::currentThread());
	bool exists = ( mRenderCommandBuffers.count( currentFramebuffer ) > 0 );
	bool changed = ( exists and ( currentFramebuffer->hash() != mFramebuffersHashes[currentFramebuffer] ) );

	if ( mDescriptorsChanged or exists == false or changed ) {
		VkCommandBuffer commandBuffer;

		if ( exists == false ) {
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = mInstance->commandPool();
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;
			if ( vkAllocateCommandBuffers( mInstance->device(), &allocInfo, &commandBuffer ) != VK_SUCCESS ) {
				throw std::runtime_error("failed to allocate command buffers!");
			}
		} else {
			commandBuffer = mRenderCommandBuffers[currentFramebuffer];
			vkResetCommandBuffer( commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
		}

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		if ( vkBeginCommandBuffer( commandBuffer, &beginInfo ) != VK_SUCCESS ) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkBufferCopy copyRegion = {};
		copyRegion.size = mIndirectBuffer.size;
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		vkCmdCopyBuffer( commandBuffer, mIndirectBuffer.host.buffer, mIndirectBuffer.device.buffer, 1, &copyRegion );
		copyRegion.size = mVertexBuffer.size;
		vkCmdCopyBuffer( commandBuffer, mVertexBuffer.host.buffer, mVertexBuffer.device.buffer, 1, &copyRegion );

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mInstance->renderPass()->renderPass();
		renderPassInfo.framebuffer = currentFramebuffer->framebuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = VkExtent2D{ (uint32_t)currentFramebuffer->viewport().width, (uint32_t)currentFramebuffer->viewport().height };

		vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
			vkCmdSetViewport( commandBuffer, 0, 1, &currentFramebuffer->viewport() );
			vkCmdSetScissor( commandBuffer, 0, 1, &currentFramebuffer->scissors() );
			vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline );
			vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSet, 0, nullptr );
			vkCmdBindVertexBuffers( commandBuffer, 0, 1, &mVertexBuffer.device.buffer, offsets );
			vkCmdDrawIndirect( commandBuffer, mIndirectBuffer.device.buffer, 0, 1, sizeof(VkDrawIndirectCommand) );
		vkCmdEndRenderPass( commandBuffer );

		if ( vkEndCommandBuffer( commandBuffer ) != VK_SUCCESS ) {
			throw std::runtime_error("failed to record command buffer!");
		}
		mRenderCommandBuffers[currentFramebuffer] = commandBuffer;
		mFramebuffersHashes[currentFramebuffer] = currentFramebuffer->hash();
	}

	void* data;
	vkMapMemory( mInstance->device(), mIndirectBuffer.host.memory, 0, mIndirectBuffer.size, 0, &data );
	((VkDrawIndirectCommand*)data)[0] = { (uint32_t)n, 1, 0, 0 };
	vkUnmapMemory( mInstance->device(), mIndirectBuffer.host.memory );

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
/*
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &currentFramebuffer->imageAvailableSemaphores();
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &currentFramebuffer->imageAvailableSemaphores();
*/
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mRenderCommandBuffers[currentFramebuffer];
	if ( vkQueueSubmit( mInstance->graphicsQueue(), 1, &submitInfo, mSingleDrawsFence ) != VK_SUCCESS ) {
		std::cerr << "failed to submit draw command buffer" << std::endl;
		exit(1);
	}
}


void VulkanRenderer2D::Draw( int x, int y, Image* image, int tx, int ty, int tw, int th, float angle )
{
	Prerender( image );

	if ( tw == -1 ) {
		tw = image->width();
	}
	if ( th == -1 ) {
		th = image->height();
	}

	Vertex vertices[6];
	memset( vertices, 0, sizeof(vertices) );
	vertices[0].u = (float)tx / image->width();
	vertices[0].v = (float)ty / image->height();
	vertices[0].x = x;
	vertices[0].y = y;

	vertices[1].u = (float)(tx+tw) / image->width();
	vertices[1].v = (float)(ty+th) / image->height();
	vertices[1].x = x + image->width();
	vertices[1].y = y + image->height();

	vertices[2].u = (float)(tx+tw) / image->width();
	vertices[2].v = (float)ty / image->height();
	vertices[2].x = x + image->width();
	vertices[2].y = y;

	vertices[3].u = (float)tx / image->width();
	vertices[3].v = (float)ty / image->height();
	vertices[3].x = x;
	vertices[3].y = y;

	vertices[4].u = (float)tx / image->width();
	vertices[4].v = (float)(ty+th) / image->height();
	vertices[4].x = x;
	vertices[4].y = y + image->height();

	vertices[5].u = (float)(tx+tw) / image->width();
	vertices[5].v = (float)(ty+th) / image->height();
	vertices[5].x = x + image->width();
	vertices[5].y = y + image->height();

	vertices[0].color[0] = vertices[1].color[0] = vertices[2].color[0] = vertices[3].color[0] = vertices[4].color[0] = vertices[5].color[0] = 1.0f;
	vertices[0].color[1] = vertices[1].color[1] = vertices[2].color[1] = vertices[3].color[1] = vertices[4].color[1] = vertices[5].color[1] = 1.0f;
	vertices[0].color[2] = vertices[1].color[2] = vertices[2].color[2] = vertices[3].color[2] = vertices[4].color[2] = vertices[5].color[2] = 1.0f;
	vertices[0].color[3] = vertices[1].color[3] = vertices[2].color[3] = vertices[3].color[3] = vertices[4].color[3] = vertices[5].color[3] = 1.0f;
	vertices[0].texid = vertices[1].texid = vertices[2].texid = vertices[3].texid = vertices[4].texid = vertices[5].texid = (float)mImageInfos[image].first;

	void* data;
	vkMapMemory( mInstance->device(), mVertexBuffer.host.memory, 0, Vertex::vertexDefinition().size() * 6, 0, &data );
	memcpy( data, vertices, Vertex::vertexDefinition().size() * 6 );
	vkUnmapMemory( mInstance->device(), mVertexBuffer.host.memory );

	Matrix m;
	if ( angle != 0.0f ) {
		m.Translate( x + image->width() / 2, y + image->height() / 2, 0.0f );
		m.RotateZ( -angle );
		m.Translate( -x - image->width() / 2, -y - image->height() / 2, 0.0f );
	}

	Render( image, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, 6, m );
}


void VulkanRenderer2D::Draw( int x, int y, int w, int h, Image* image, int tx, int ty, int tw, int th, float angle )
{
	Prerender( image );

	if ( tw == -1 ) {
		tw = image->width();
	}
	if ( th == -1 ) {
		th = image->height();
	}

	Vertex vertices[6];
	memset( vertices, 0, sizeof(vertices) );
	vertices[0].u = (float)tx / image->width();
	vertices[0].v = (float)ty / image->height();
	vertices[0].x = x;
	vertices[0].y = y;

	vertices[1].u = (float)(tx+tw) / image->width();
	vertices[1].v = (float)(ty+th) / image->height();
	vertices[1].x = x + w;
	vertices[1].y = y + h;

	vertices[2].u = (float)(tx+tw) / image->width();
	vertices[2].v = (float)ty / image->height();
	vertices[2].x = x + w;
	vertices[2].y = y;

	vertices[3].u = (float)tx / image->width();
	vertices[3].v = (float)ty / image->height();
	vertices[3].x = x;
	vertices[3].y = y;

	vertices[4].u = (float)tx / image->width();
	vertices[4].v = (float)(ty+th) / image->height();
	vertices[4].x = x;
	vertices[4].y = y + h;

	vertices[5].u = (float)(tx+tw) / image->width();
	vertices[5].v = (float)(ty+th) / image->height();
	vertices[5].x = x + w;
	vertices[5].y = y + h;

	vertices[0].color[0] = vertices[1].color[0] = vertices[2].color[0] = vertices[3].color[0] = vertices[4].color[0] = vertices[5].color[0] = 1.0f;
	vertices[0].color[1] = vertices[1].color[1] = vertices[2].color[1] = vertices[3].color[1] = vertices[4].color[1] = vertices[5].color[1] = 1.0f;
	vertices[0].color[2] = vertices[1].color[2] = vertices[2].color[2] = vertices[3].color[2] = vertices[4].color[2] = vertices[5].color[2] = 1.0f;
	vertices[0].color[3] = vertices[1].color[3] = vertices[2].color[3] = vertices[3].color[3] = vertices[4].color[3] = vertices[5].color[3] = 1.0f;
	vertices[0].texid = vertices[1].texid = vertices[2].texid = vertices[3].texid = vertices[4].texid = vertices[5].texid = (float)mImageInfos[image].first;

	void* data;
	vkMapMemory( mInstance->device(), mVertexBuffer.host.memory, 0, Vertex::vertexDefinition().size() * 6, 0, &data );
	memcpy( data, vertices, Vertex::vertexDefinition().size() * 6 );
	vkUnmapMemory( mInstance->device(), mVertexBuffer.host.memory );

	Matrix m;
	if ( angle != 0.0f ) {
		m.Translate( x + image->width() / 2, y + image->height() / 2, 0.0f );
		m.RotateZ( -angle );
		m.Translate( -x - image->width() / 2, -y - image->height() / 2, 0.0f );
	}

	Render( image, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, 6, m );
}


void VulkanRenderer2D::DrawText( int x, int y, Font* font, uint32_t color, const std::string& text, Renderer2D::TextFlags flags )
{
	Prerender( font->texture() );

	y += font->size();
	const int base_x = x;
	const char* data = text.data();
	const uint32_t len = text.length();

	const float rx = 1.0f / font->texture()->width();
	const float ry = 1.0f / font->texture()->height();
	uint32_t iBuff = 0;

	uint32_t* lines_ofs = nullptr;
	uint32_t line = 0;

	if ( flags & Renderer2D::HCenter ) {
		uint32_t width = 0;
		for ( uint32_t i = 0; i < len; i++ ) {
			if ( data[i] == '\n' ) {
				line++;
			}
		}
		lines_ofs = new uint32_t[ ++line ];
		line = 0;
		for ( uint32_t i = 0; i < len; i++ ) {
			int _c = data[i];
			width += font->glyph( _c )->advX;

			if ( data[i] == '\n' ) {
				lines_ofs[line] = x - width / 2;
				line++;
				width = 0;
			}
		}
		lines_ofs[line] = x - width / 2;
		line = 0;
		x = lines_ofs[0];
	}

	Matrix m;
	Vertex vertices[6*32];
	memset( vertices, 0, sizeof(vertices) );

	for ( uint32_t i = 0; i < len; i++ ) {
		int _c = data[i];
		uint8_t c = _c;

		if ( data[i] == '\n' ) {
			x = lines_ofs ? lines_ofs[++line] : base_x;
			y += font->size();
			continue;
		}

		float sx = ( (float)font->glyph(c)->x ) * rx;
		float sy = ( (float)font->glyph(c)->y ) * ry;
		float texMaxX = ( (float)font->glyph(c)->w ) * rx;
		float texMaxY = ( (float)font->glyph(c)->h ) * ry;
		float width = font->glyph(c)->w;
		float height = font->glyph(c)->h;
		float fy = (float)y - font->glyph(c)->posY;

		vertices[iBuff + 0].u = sx;
		vertices[iBuff + 0].v = sy;
		vertices[iBuff + 0].x = x;
		vertices[iBuff + 0].y = fy;
// 		vertices[iBuff + 0].z = 0.0f;

		vertices[iBuff + 1].u = sx+texMaxX;
		vertices[iBuff + 1].v = sy+texMaxY;
		vertices[iBuff + 1].x = x+width;
		vertices[iBuff + 1].y = fy+height;
// 		vertices[iBuff + 1].z = 0.0f;

		vertices[iBuff + 2].u = sx+texMaxX;
		vertices[iBuff + 2].v = sy;
		vertices[iBuff + 2].x = x+width;
		vertices[iBuff + 2].y = fy;
// 		vertices[iBuff + 2].z = 0.0f;
		
		vertices[iBuff + 3].u = sx;
		vertices[iBuff + 3].v = sy;
		vertices[iBuff + 3].x = x;
		vertices[iBuff + 3].y = fy;
// 		vertices[iBuff + 3].z = 0.0f;

		vertices[iBuff + 4].u = sx;
		vertices[iBuff + 4].v = sy+texMaxY;
		vertices[iBuff + 4].x = x;
		vertices[iBuff + 4].y = fy+height;
// 		vertices[iBuff + 4].z = 0.0f;

		vertices[iBuff + 5].u = sx+texMaxX;
		vertices[iBuff + 5].v = sy+texMaxY;
		vertices[iBuff + 5].x = x+width;
		vertices[iBuff + 5].y = fy+height;
// 		vertices[iBuff + 5].z = 0.0f;

		vertices[iBuff + 0].color[0] = vertices[iBuff + 1].color[0] = vertices[iBuff + 2].color[0] = vertices[iBuff + 3].color[0] = vertices[iBuff + 4].color[0] = vertices[iBuff + 5].color[0] = ( (float)( color & 0xFF ) ) / 255.0f;
		vertices[iBuff + 0].color[1] = vertices[iBuff + 1].color[1] = vertices[iBuff + 2].color[1] = vertices[iBuff + 3].color[1] = vertices[iBuff + 4].color[1] = vertices[iBuff + 5].color[1] = ( (float)( ( color >> 8 ) & 0xFF ) ) / 255.0f;
		vertices[iBuff + 0].color[2] = vertices[iBuff + 1].color[2] = vertices[iBuff + 2].color[2] = vertices[iBuff + 3].color[2] = vertices[iBuff + 4].color[2] = vertices[iBuff + 5].color[2] = ( (float)( ( color >> 16 ) & 0xFF ) ) / 255.0f;
		vertices[iBuff + 0].color[3] = vertices[iBuff + 1].color[3] = vertices[iBuff + 2].color[3] = vertices[iBuff + 3].color[3] = vertices[iBuff + 4].color[3] = vertices[iBuff + 5].color[3] = ( (float)( ( color >> 24 ) & 0xFF ) ) / 255.0f;
/*
		vertices[iBuff + 0].color[0] = vertices[iBuff + 1].color[0] = vertices[iBuff + 2].color[0] = vertices[iBuff + 3].color[0] = vertices[iBuff + 4].color[0] = vertices[iBuff + 5].color[0] = 1.0;
		vertices[iBuff + 0].color[1] = vertices[iBuff + 1].color[1] = vertices[iBuff + 2].color[1] = vertices[iBuff + 3].color[1] = vertices[iBuff + 4].color[1] = vertices[iBuff + 5].color[1] = 1.0;
		vertices[iBuff + 0].color[2] = vertices[iBuff + 1].color[2] = vertices[iBuff + 2].color[2] = vertices[iBuff + 3].color[2] = vertices[iBuff + 4].color[2] = vertices[iBuff + 5].color[2] = 1.0;
		vertices[iBuff + 0].color[3] = vertices[iBuff + 1].color[3] = vertices[iBuff + 2].color[3] = vertices[iBuff + 3].color[3] = vertices[iBuff + 4].color[3] = vertices[iBuff + 5].color[3] = 1.0;
*/
		vertices[iBuff + 0].texid = vertices[iBuff + 1].texid = vertices[iBuff + 2].texid = vertices[iBuff + 3].texid = vertices[iBuff + 4].texid = vertices[iBuff + 5].texid = (float)mImageInfos[font->texture()].first;

		x += font->glyph(c)->advX;
		iBuff += 6;
		if ( iBuff >= 6*32 && i + 1 < len ) {
			void* vkdata;
			vkMapMemory( mInstance->device(), mVertexBuffer.host.memory, 0, Vertex::vertexDefinition().size() * iBuff, 0, &vkdata );
			memcpy( vkdata, vertices, Vertex::vertexDefinition().size() * iBuff );
			vkUnmapMemory( mInstance->device(), mVertexBuffer.host.memory );
			Render( font->texture(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, iBuff, m );
			iBuff = 0;
		}
	}

	void* vkdata;
	vkMapMemory( mInstance->device(), mVertexBuffer.host.memory, 0, Vertex::vertexDefinition().size() * iBuff, 0, &vkdata );
	memcpy( vkdata, vertices, Vertex::vertexDefinition().size() * iBuff );
	vkUnmapMemory( mInstance->device(), mVertexBuffer.host.memory );
	Render( font->texture(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, iBuff, m );

	if ( lines_ofs ) {
		delete lines_ofs;
	}
}


void VulkanRenderer2D::DrawText( int x, int y, Font* font, uint32_t color, const std::wstring& text, Renderer2D::TextFlags flags )
{
	const wchar_t* data = text.c_str();
	const uint32_t len = text.length();
	bool rerender = false;
	std::map< wchar_t, Font::Glyph >& glyphs = font->glyphs();

	for ( uint32_t i = 0; i < len; i++ ) {
		wchar_t c = data[i];
		if ( glyphs.count( c ) <= 0 ) {
			Font::Glyph g = {
				.c = c,
				.x = 0,
				.y = 0,
				.w = 0,
				.h = 0,
				.advX = 0,
				.posY = 0
			};
			glyphs.insert( std::make_pair( c, g ) );
			rerender = true;
		}
	}

	if ( rerender ) {
		font->RenderGlyphs();
	}

	Prerender( font->texture() );

	y += font->size();
	const int base_x = x;
	const float rx = 1.0f / font->texture()->width();
	const float ry = 1.0f / font->texture()->height();
	uint32_t iBuff = 0;

	uint32_t* lines_ofs = nullptr;
	uint32_t line = 0;

	if ( flags & Renderer2D::HCenter ) {
		uint32_t width = 0;
		for ( uint32_t i = 0; i < len; i++ ) {
			if ( data[i] == '\n' ) {
				line++;
			}
		}
		lines_ofs = new uint32_t[ ++line ];
		line = 0;
		for ( uint32_t i = 0; i < len; i++ ) {
			int _c = data[i];
			width += font->glyph( _c )->advX;

			if ( data[i] == '\n' ) {
				lines_ofs[line] = x - width / 2;
				line++;
				width = 0;
			}
		}
		lines_ofs[line] = x - width / 2;
		line = 0;
		x = lines_ofs[0];
	}

	Matrix m;
	Vertex vertices[6*32];
	memset( vertices, 0, sizeof(vertices) );

	for ( uint32_t i = 0; i < len; i++ ) {
		wchar_t c = text[i];

		if ( data[i] == '\n' ) {
			x = lines_ofs ? lines_ofs[++line] : base_x;
			y += font->size();
			continue;
		}

		float sx = ( (float)font->glyph(c)->x ) * rx;
		float sy = ( (float)font->glyph(c)->y ) * ry;
		float texMaxX = ( (float)font->glyph(c)->w ) * rx;
		float texMaxY = ( (float)font->glyph(c)->h ) * ry;
		float width = font->glyph(c)->w;
		float height = font->glyph(c)->h;
		float fy = (float)y - font->glyph(c)->posY;

		vertices[iBuff + 0].u = sx;
		vertices[iBuff + 0].v = sy;
		vertices[iBuff + 0].x = x;
		vertices[iBuff + 0].y = fy;
// 		vertices[iBuff + 0].z = 0.0f;

		vertices[iBuff + 1].u = sx+texMaxX;
		vertices[iBuff + 1].v = sy+texMaxY;
		vertices[iBuff + 1].x = x+width;
		vertices[iBuff + 1].y = fy+height;
// 		vertices[iBuff + 1].z = 0.0f;

		vertices[iBuff + 2].u = sx+texMaxX;
		vertices[iBuff + 2].v = sy;
		vertices[iBuff + 2].x = x+width;
		vertices[iBuff + 2].y = fy;
// 		vertices[iBuff + 2].z = 0.0f;
		
		vertices[iBuff + 3].u = sx;
		vertices[iBuff + 3].v = sy;
		vertices[iBuff + 3].x = x;
		vertices[iBuff + 3].y = fy;
// 		vertices[iBuff + 3].z = 0.0f;

		vertices[iBuff + 4].u = sx;
		vertices[iBuff + 4].v = sy+texMaxY;
		vertices[iBuff + 4].x = x;
		vertices[iBuff + 4].y = fy+height;
// 		vertices[iBuff + 4].z = 0.0f;

		vertices[iBuff + 5].u = sx+texMaxX;
		vertices[iBuff + 5].v = sy+texMaxY;
		vertices[iBuff + 5].x = x+width;
		vertices[iBuff + 5].y = fy+height;
// 		vertices[iBuff + 5].z = 0.0f;

		vertices[iBuff + 0].color[0] = vertices[iBuff + 1].color[0] = vertices[iBuff + 2].color[0] = vertices[iBuff + 3].color[0] = vertices[iBuff + 4].color[0] = vertices[iBuff + 5].color[0] = ( (float)( color & 0xFF ) ) / 255.0f;
		vertices[iBuff + 0].color[1] = vertices[iBuff + 1].color[1] = vertices[iBuff + 2].color[1] = vertices[iBuff + 3].color[1] = vertices[iBuff + 4].color[1] = vertices[iBuff + 5].color[1] = ( (float)( ( color >> 8 ) & 0xFF ) ) / 255.0f;
		vertices[iBuff + 0].color[2] = vertices[iBuff + 1].color[2] = vertices[iBuff + 2].color[2] = vertices[iBuff + 3].color[2] = vertices[iBuff + 4].color[2] = vertices[iBuff + 5].color[2] = ( (float)( ( color >> 16 ) & 0xFF ) ) / 255.0f;
		vertices[iBuff + 0].color[3] = vertices[iBuff + 1].color[3] = vertices[iBuff + 2].color[3] = vertices[iBuff + 3].color[3] = vertices[iBuff + 4].color[3] = vertices[iBuff + 5].color[3] = ( (float)( ( color >> 24 ) & 0xFF ) ) / 255.0f;
/*
		vertices[iBuff + 0].color[0] = vertices[iBuff + 1].color[0] = vertices[iBuff + 2].color[0] = vertices[iBuff + 3].color[0] = vertices[iBuff + 4].color[0] = vertices[iBuff + 5].color[0] = 1.0;
		vertices[iBuff + 0].color[1] = vertices[iBuff + 1].color[1] = vertices[iBuff + 2].color[1] = vertices[iBuff + 3].color[1] = vertices[iBuff + 4].color[1] = vertices[iBuff + 5].color[1] = 1.0;
		vertices[iBuff + 0].color[2] = vertices[iBuff + 1].color[2] = vertices[iBuff + 2].color[2] = vertices[iBuff + 3].color[2] = vertices[iBuff + 4].color[2] = vertices[iBuff + 5].color[2] = 1.0;
		vertices[iBuff + 0].color[3] = vertices[iBuff + 1].color[3] = vertices[iBuff + 2].color[3] = vertices[iBuff + 3].color[3] = vertices[iBuff + 4].color[3] = vertices[iBuff + 5].color[3] = 1.0;
*/
		vertices[iBuff + 0].texid = vertices[iBuff + 1].texid = vertices[iBuff + 2].texid = vertices[iBuff + 3].texid = vertices[iBuff + 4].texid = vertices[iBuff + 5].texid = (float)mImageInfos[font->texture()].first;

		x += font->glyph(c)->advX;
		iBuff += 6;

		if ( iBuff >= 6*32 && i + 1 < len ) {
			void* vkdata;
			vkMapMemory( mInstance->device(), mVertexBuffer.host.memory, 0, Vertex::vertexDefinition().size() * iBuff, 0, &vkdata );
			memcpy( vkdata, vertices, Vertex::vertexDefinition().size() * iBuff );
			vkUnmapMemory( mInstance->device(), mVertexBuffer.host.memory );
			Render( font->texture(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, iBuff, m );
			iBuff = 0;
		}
	}

	void* vkdata;
	vkMapMemory( mInstance->device(), mVertexBuffer.host.memory, 0, Vertex::vertexDefinition().size() * iBuff, 0, &vkdata );
	memcpy( vkdata, vertices, Vertex::vertexDefinition().size() * iBuff );
	vkUnmapMemory( mInstance->device(), mVertexBuffer.host.memory );
	Render( font->texture(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, iBuff, m );
	if ( lines_ofs ) {
		delete lines_ofs;
	}
}


void VulkanRenderer2D::DrawLine( int x0, int y0, uint32_t color0, int x1, int y1, uint32_t color1 )
{
	Prerender( mTextureWhite );

	Matrix matrix;
	Vertex vertices[6*2];
	memset( vertices, 0, sizeof(vertices) );

	vertices[0].x = x0;
	vertices[0].y = y0;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;
	vertices[0].color[0] = ( (float)( color0 & 0xFF ) ) / 255.0f;
	vertices[0].color[1] = ( (float)( ( color0 >> 8 ) & 0xFF ) ) / 255.0f;
	vertices[0].color[2] = ( (float)( ( color0 >> 16 ) & 0xFF ) ) / 255.0f;
	vertices[0].color[3] = ( (float)( ( color0 >> 24 ) & 0xFF ) ) / 255.0f;

	vertices[1].x = x1;
	vertices[1].y = y1;
	vertices[1].u = 0.0f;
	vertices[1].v = 0.0f;
	vertices[1].color[0] = ( (float)( color1 & 0xFF ) ) / 255.0f;
	vertices[1].color[1] = ( (float)( ( color1 >> 8 ) & 0xFF ) ) / 255.0f;
	vertices[1].color[2] = ( (float)( ( color1 >> 16 ) & 0xFF ) ) / 255.0f;
	vertices[1].color[3] = ( (float)( ( color1 >> 24 ) & 0xFF ) ) / 255.0f;

	vertices[0].texid = vertices[1].texid = (float)mImageInfos[mTextureWhite].first;

// 	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
// 	glBufferSubData( GL_ARRAY_BUFFER, 6 * sizeof(Vertex2D), 2 * sizeof(Vertex2D), vertices );
	Render( mTextureWhite, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 6, 2, matrix );
}


uintptr_t VulkanRenderer2D::attributeID( const std::string& name )
{
	if ( !m2DReady ) {
		Compute();
	}
// 	return glGetAttribLocation( mShader, name.c_str() );
}


uintptr_t VulkanRenderer2D::uniformID( const std::string& name )
{
	if ( !m2DReady ) {
		Compute();
	}
// 	return glGetUniformLocation( mShader, name.c_str() );
}


void VulkanRenderer2D::uniformUpload( const uintptr_t id, const float f )
{
// 	glUseProgram( mShader );
// 	glUniform1f( id, f );
}


void VulkanRenderer2D::uniformUpload( const uintptr_t id, const Vector2f& v )
{
// 	glUseProgram( mShader );
// 	glUniform2f( id, v.x, v.y );
}


void VulkanRenderer2D::uniformUpload( const uintptr_t id, const Vector3f& v )
{
// 	glUseProgram( mShader );
// 	glUniform3f( id, v.x, v.y, v.z );
}


void VulkanRenderer2D::uniformUpload( const uintptr_t id, const Vector4f& v )
{
// 	glUseProgram( mShader );
// 	glUniform4f( id, v.x, v.y, v.z, v.w );
}


void VulkanRenderer2D::uniformUpload( const uintptr_t id, const Matrix& v )
{
// 	glUseProgram( mShader );
// 	glUniformMatrix4fv( id, 1, GL_FALSE, v.constData() );
}
