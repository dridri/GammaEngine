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

#include "FbWindow.h"
#include "FbInstance.h"
#include "Debug.h"

extern "C" GE::Window* CreateWindow( GE::Instance* instance, const std::string& title, int width, int height, FbWindow::Flags flags ) {
	return (GE::Window*)new FbWindow( instance, title, width, height, (Window::Flags)flags );
}

FbWindow::FbWindow( Instance* instance, const std::string& title, int width, int height, Window::Flags flags )
	: ProxyWindow< GE::FramebufferWindow >( instance, title, width, height, (ProxyWindow< GE::FramebufferWindow >::Flags)flags )
	, mClearColor( 0 )
{
	FbInstance* fbinst = dynamic_cast< FbInstance* >( instance );
	if ( fbinst->boundWindow() == nullptr ) {
		fbinst->BindWindow( this );
	}
}


FbWindow::~FbWindow()
{
}


uint64_t FbWindow::colorImage()
{
	return 0;
}


void FbWindow::Clear( uint32_t color )
{
	ProxyWindow< GE::FramebufferWindow >::ClearRegion( color );
}


void FbWindow::BindTarget()
{
}


void FbWindow::SwapBuffers()
{
	SwapBuffersBase();
}


uint64_t FbWindow::CreateSharedContext()
{
	return 0;
}


void FbWindow::BindSharedContext( uint64_t ctx )
{
}


void FbWindow::ReadKeys( bool* keys )
{
	memcpy( keys, mKeys, sizeof( mKeys ) );
}
