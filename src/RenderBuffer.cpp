/*
 * The GammaEngine Library 2.0 is a multiplatform Vulkan-based game engine
 * Copyright (C) 2017  Adrien Aubry <dridri85@gmail.com>
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

#include "RenderBuffer.h"
#include "Instance.h"
#include "Window.h"

using namespace GE;

RenderBuffer::RenderBuffer( uint32_t width, uint32_t height )
	: mAssociatedWindow( nullptr )
	, mWidth( width )
	, mHeight( height )
	, mColorBuffer( new Image( width, height, 0x00000000 ) )
	, mDepthBuffer( new Image( width, height, 0x00000000 ) )
{
}


RenderBuffer::~RenderBuffer()
{
}


void RenderBuffer::AssociateSize( Window* window )
{
	mAssociatedWindow = window;
}


Image* RenderBuffer::colorBuffer()
{
	return mColorBuffer;
}


Image* RenderBuffer::depthBuffer()
{
	return mDepthBuffer;
}
