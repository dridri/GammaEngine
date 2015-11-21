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

#define BASEWINDOW_CPP

#include <stdlib.h>
#include <string.h>
#include "ios/BaseWindow.h"
#include "Window.h"
#include "Instance.h"
#include "include/Time.h"
#include "Input.h"
#include "Debug.h"

void _ge_iOSSwapBuffer( Vector2i& mCursor, Vector2i& mCursorWarp );
void _ge_iOSOpenWindow( int* width, int* height, Window::Flags flags );
int _ge_iOSStatusBarHeight();
void _ge_iOSSetAdmobPublisherID( const char* id );
void _ge_iOSShowInterstitialAd();

namespace GE {

bool BaseWindow::mKeys[512] = { false };
uint32_t BaseWindow::mBaseWidth = 0;
uint32_t BaseWindow::mBaseHeight = 0;


BaseWindow::BaseWindow( Instance* instance, const std::string& title, int width, int height, uint32_t _flags )
	: mInstance( instance )
	, mWidth( width )
	, mHeight( height )
	, mHasResized( false )
{
	Window::Flags flags = static_cast<Window::Flags>( _flags );

	_ge_iOSOpenWindow( &width, &height, flags );

	mWidth = width;
	mHeight = height;
	mBaseWidth = mWidth;
	mBaseHeight = mHeight;
}


BaseWindow::~BaseWindow()
{
}


uint32_t BaseWindow::width()
{
	return mWidth;
}


uint32_t BaseWindow::height()
{
	return mHeight;
}


Vector2i& BaseWindow::cursor()
{
	return mCursor;
}


Vector2i& BaseWindow::cursorWarp()
{
	return mCursorWarp;
}


float BaseWindow::fps() const
{
	return mFps;
}


void BaseWindow::SwapBuffersBase()
{
	mFpsImages++;
	if ( Time::GetTick() - mFpsTimer > 500 ) {
		mFps = mFpsImages * 1000.0f / ( Time::GetTick() - mFpsTimer );
		mFpsTimer = Time::GetTick();
		mFpsImages = 0;
	}

	_ge_iOSSwapBuffer( mCursor, mCursorWarp );
}


int BaseWindow::iOSWindowWidth()
{
	return mBaseWidth;
}


int BaseWindow::iOSWindowHeight()
{
	return mBaseHeight;
}


bool* BaseWindow::iOSKeys()
{
	return mKeys;
}


int BaseWindow::statusBarHeight()
{
	return _ge_iOSStatusBarHeight();
}


void BaseWindow::SetAdmobPublisherID( const std::string& id )
{
	fDebug( id );
	_ge_iOSSetAdmobPublisherID( id.c_str() );
}


void BaseWindow::ShowInterstitialAd()
{
	_ge_iOSShowInterstitialAd();
}

} // namespace GE 
