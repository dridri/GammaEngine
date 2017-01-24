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

#include "FbInstance.h"
#include "FbWindow.h"
#include "Vertex.h"
#include "Image.h"
#include "Debug.h"


extern "C" GE::Instance* CreateInstance( void* pBackend, const char* appName, uint32_t appVersion ) {
	return new FbInstance( pBackend, appName, appVersion );
}


FbInstance::FbInstance( void* pBackend, const char* appName, uint32_t appVersion )
	: Instance( pBackend )
	, mBoundWindow( nullptr )
{
	fDebug( appName, appVersion );

	EnumerateGpus();
}


int FbInstance::EnumerateGpus()
{
	fDebug();

	mGpuCount = 0;
	return mGpuCount;
}


Instance* FbInstance::CreateDevice( int devid, int queueCount )
{
	FbInstance* ret = (FbInstance*)malloc( sizeof(FbInstance) );
	memset( (void*)ret, 0, sizeof(FbInstance) );
	memcpy( (void*)ret, (void*)this, sizeof(FbInstance) );
	ret->mCpuRamCounter = 0;
	ret->mGpuRamCounter = 0;
	ret->mDevId = devid;

	fDebug( devid, queueCount );

	return ret;
}


FbWindow* FbInstance::boundWindow() const
{
	return mBoundWindow;
}


void FbInstance::BindWindow( FbWindow* win )
{
	mBoundWindow = win;
}


uint64_t FbInstance::ReferenceImage( Image* image )
{
	return (uint64_t)image;
}


void FbInstance::UnreferenceImage( uint64_t ref )
{
	(void)ref;
}


void FbInstance::UpdateImageData( Image* image, uint64_t ref )
{
}


void FbInstance::AffectVRAM( int64_t sz )
{
	mGpuRamCounter += sz;
}
