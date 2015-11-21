/*
 * The GammaEngine Library 2.0 is a multiplatform Vulkan-based game engine
 * Copyright (C) 2015  Adrien Aubry <dridri85@gmail.com>
 *
 * This provkam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This provkam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this provkam.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>
#include "VulkanWindow.h"
#include "VulkanInstance.h"

extern "C" GE::Window* CreateWindow( GE::Instance* instance, const std::string& title, int width, int height, VulkanWindow::Flags flags ) {
	return new VulkanWindow( instance, title, width, height, flags );
}

VulkanWindow::VulkanWindow( Instance* instance, const std::string& title, int width, int height, Flags flags )
	: Window( instance, title, width, height, flags )
	, mColorImage( 0 )
	, mColorImageMemRef( {} )
	, mImageColorRange( {} )
	, mColorTargetView( 0 )
	, mDepthImage( 0 )
	, mDepthImageMemRef( {} )
	, mImageDepthRange( {} )
	, mDepthTargetView( 0 )
	, mBindCmdBuffer( 0 )
	, mClearCmdBuffer( 0 )
	, mClearColor( 0 )
{

#ifdef GE_WIN32
#elif defined(GE_LINUX)
	VK_CONNECTION_INFO connectionInfo;
	connectionInfo.dpy = mDisplay;
	connectionInfo.screen = mScreen;
	connectionInfo.window = mWindow;
	vkWsiX11AssociateConnection( instance->gpu(), &connectionInfo );
#endif

	InitPresentableImage();
}


VulkanWindow::~VulkanWindow()
{
}


VK_IMAGE VulkanWindow::colorImage()
{
	return mColorImage;
}


void VulkanWindow::InitPresentableImage()
{
	VK_WSI_WIN_PRESENTABLE_IMAGE_CREATE_INFO imageCreateInfo = {};
	imageCreateInfo.format = {
		VK_CH_FMT_R8G8B8A8,
		VK_NUM_FMT_UNORM
	};
	imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_TARGET;
	imageCreateInfo.extent = { (int)mWidth, (int)mHeight };
	vkWsiWinCreatePresentableImage( mInstance->device(), &imageCreateInfo, &mColorImage, &mColorImageMemRef.mem );

	imageCreateInfo.format = {
		VK_CH_FMT_R8,
		VK_NUM_FMT_UNDEFINED
	};
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL;
	imageCreateInfo.extent = { (int)mWidth, (int)mHeight };
	vkWsiWinCreatePresentableImage( mInstance->device(), &imageCreateInfo, &mDepthImage, &mDepthImageMemRef.mem );

	mImageColorRange.aspect = VK_IMAGE_ASPECT_COLOR;
	mImageColorRange.baseMipLevel = 0;
	mImageColorRange.mipLevels = 1;
	mImageColorRange.baseArraySlice = 0;
	mImageColorRange.arraySize = 1;

	mImageDepthRange.aspect = VK_IMAGE_ASPECT_DEPTH;
	mImageDepthRange.baseMipLevel = 0;
	mImageDepthRange.mipLevels = 1;
	mImageDepthRange.baseArraySlice = 0;
	mImageDepthRange.arraySize = 1;

	VK_CMD_BUFFER initCmdBuffer;
	VK_CMD_BUFFER_CREATE_INFO bufferCreateInfo = { 0, 0 };
	vkCreateCommandBuffer( mInstance->device(), &bufferCreateInfo, &initCmdBuffer );

	vkBeginCommandBuffer( initCmdBuffer, 0 );
		VK_IMAGE_STATE_TRANSITION initTransition = {};
		initTransition.image = mColorImage;
		initTransition.oldState = VK_MEMORY_STATE_UNINITIALIZED;
		initTransition.newState = VK_WSI_WIN_IMAGE_STATE_PRESENT_WINDOWED;
		initTransition.subresourceRange = mImageColorRange;
		vkCmdPrepareImages( initCmdBuffer, 1, &initTransition );
		VK_IMAGE_STATE_TRANSITION initTransition2 = {};
		initTransition.image = mColorImage;
		initTransition.oldState = VK_MEMORY_STATE_UNINITIALIZED;
		initTransition.newState = VK_MEMORY_STATE_GRAPHICS_SHADER_READ_WRITE; // TESTING
		initTransition.subresourceRange = mImageDepthRange;
		vkCmdPrepareImages( initCmdBuffer, 1, &initTransition2 );
	vkEndCommandBuffer( initCmdBuffer );

	VK_MEMORY_REF refs[2] = { mColorImageMemRef, mDepthImageMemRef };
	((VulkanInstance*)mInstance)->QueueSubmit( initCmdBuffer, refs, 2 );



	VK_COLOR_TARGET_VIEW_CREATE_INFO colorTargetViewCreateInfo = {};
	colorTargetViewCreateInfo.image = mColorImage;
	colorTargetViewCreateInfo.arraySize = 1;
	colorTargetViewCreateInfo.baseArraySlice = 0;
	colorTargetViewCreateInfo.mipLevel = 0;
	colorTargetViewCreateInfo.format.channelFormat = VK_CH_FMT_R8G8B8A8;
	colorTargetViewCreateInfo.format.numericFormat = VK_NUM_FMT_UNORM;
	vkCreateColorTargetView( mInstance->device(), &colorTargetViewCreateInfo, &mColorTargetView );

	// TODO : vkCreateDepthStencilView
}


void VulkanWindow::Clear( uint32_t color )
{
	VK_CMD_BUFFER_CREATE_INFO bufferCreateInfo = { 0, 0 };

	if ( mClearCmdBuffer == 0 || color != mClearColor ){
		mClearColor = color;

		if ( mClearCmdBuffer == 0 ) {
			vkCreateCommandBuffer( mInstance->device(), &bufferCreateInfo, &mClearCmdBuffer );
		}

		vkBeginCommandBuffer( mClearCmdBuffer, 0 );
			VK_IMAGE_STATE_TRANSITION transition = {};
			transition.image = mColorImage;
			transition.oldState = VK_MEMORY_STATE_TARGET_RENDER_ACCESS_OPTIMAL;
			transition.newState = VK_MEMORY_STATE_CLEAR;
			transition.subresourceRange = mImageColorRange;
			vkCmdPrepareImages( mClearCmdBuffer, 1, &transition );
			float clearColor[] = {
				(float)( ( color >>  0 ) & 0xFF ) / 255.0f,
				(float)( ( color >>  8 ) & 0xFF ) / 255.0f,
				(float)( ( color >> 16 ) & 0xFF ) / 255.0f,
				(float)( ( color >> 24 ) & 0xFF ) / 255.0f
			};
			vkCmdClearColorImage( mClearCmdBuffer, mColorImage, clearColor, 1, &mImageColorRange );
			transition.image = mColorImage;
			transition.oldState = VK_MEMORY_STATE_CLEAR;
			transition.newState = VK_MEMORY_STATE_TARGET_RENDER_ACCESS_OPTIMAL;
			transition.subresourceRange = mImageColorRange;
			vkCmdPrepareImages( mClearCmdBuffer, 1, &transition );
		vkEndCommandBuffer( mClearCmdBuffer );
	}

	((VulkanInstance*)mInstance)->QueueSubmit( mClearCmdBuffer, &mColorImageMemRef, 1 );
}


void VulkanWindow::BindTarget()
{
	if ( mBindCmdBuffer == 0 ) {
		VK_CMD_BUFFER_CREATE_INFO info = { 0, 0 };
		vkCreateCommandBuffer( mInstance->device(), &info, &mBindCmdBuffer );
		vkBeginCommandBuffer( mBindCmdBuffer, 0 );
			VK_COLOR_TARGET_BIND_INFO colorTargetBindInfo;
			colorTargetBindInfo.view = mColorTargetView;
			colorTargetBindInfo.colorTargetState = VK_MEMORY_STATE_TARGET_RENDER_ACCESS_OPTIMAL;
			VK_DEPTH_STENCIL_BIND_INFO depthStencilBindInfo;
			depthStencilBindInfo.view = mDepthTargetView;
			depthStencilBindInfo.depthState = VK_MEMORY_STATE_TARGET_RENDER_ACCESS_OPTIMAL;
			depthStencilBindInfo.stencilState = VK_MEMORY_STATE_TARGET_RENDER_ACCESS_OPTIMAL;
			vkCmdBindTargets( mBindCmdBuffer, 1, &colorTargetBindInfo, &depthStencilBindInfo );
		vkEndCommandBuffer( mBindCmdBuffer );
	}

	((VulkanInstance*)mInstance)->QueueSubmit( mBindCmdBuffer, &mColorImageMemRef, 1 );
}


void VulkanWindow::SwapBuffers()
{
	VK_WSI_WIN_PRESENT_INFO presentInfo = {};
	presentInfo.hWndDest = mWindow;
	presentInfo.srcImage = mColorImage;
	presentInfo.presentMode = VK_WSI_WIN_PRESENT_MODE_WINDOWED;

	vkWsiWinQueuePresent( mInstance->queue(), &presentInfo );

	SwapBuffersBase();
}


uint64_t VulkanWindow::CreateSharedContext()
{
	return 0;
}


void VulkanWindow::BindSharedContext( uint64_t ctx )
{
}


void VulkanWindow::ReadKeys( bool* keys )
{
	memcpy( keys, mKeys, sizeof( mKeys ) );
}
