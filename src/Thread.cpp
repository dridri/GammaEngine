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

#include <unistd.h>
#include "Window.h"
#include "Thread.h"

using namespace GE;

Thread::Thread( Window* shared_graphics_window )
	: mSharedWindow( shared_graphics_window )
	, mSharedContext( 0 )
	, mRunning( false )
	, mIsRunning( false )
	, mFinished( false )
// 	, mThread( nullptr )
{
	if ( mSharedWindow ) {
		mSharedContext = mSharedWindow->CreateSharedContext();
	}
// 	mThread = new std::thread( &Thread::sThreadEntry, this );
	pthread_create( &mThread, nullptr, (void*(*)(void*))&Thread::sThreadEntry, this );
}


Thread::~Thread()
{
}


void Thread::Start()
{
	mRunning = true;
}


void Thread::Pause()
{
	mRunning = false;
}


void Thread::Join()
{
	while ( !mFinished ) {
		usleep( 1000 * 10 );
	}
}


bool Thread::running()
{
	return mIsRunning;
}


void Thread::sThreadEntry( Thread* thiz )
{
	thiz->mThreadEntry();
}


void Thread::mThreadEntry()
{
	if ( mSharedWindow && mSharedContext ) {
		mSharedWindow->BindSharedContext( mSharedContext );
	}
	do {
		while ( !mRunning ) {
			mIsRunning = false;
			usleep( 1000 * 10 );
		}
		mIsRunning = true;
	} while ( run() );
	mIsRunning = false;
	mFinished = true;
}
