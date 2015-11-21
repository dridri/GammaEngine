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

#include "OpenGLES20Instance.h"
#include "OpenGLES20Window.h"
#include "Vertex.h"
#include "Image.h"
#include "Debug.h"


static uint32_t geGetNextPower2( uint32_t width )
{
	uint32_t b = width;
	int n;
	for ( n = 0; b != 0; n++ ) b >>= 1;
	b = 1 << n;
	if ( b == 2 * width ) b >>= 1;
	return b;
}


extern "C" GE::Instance* CreateInstance( const char* appName, uint32_t appVersion ) {
	return new OpenGLES20Instance( appName, appVersion );
}


OpenGLES20Instance::OpenGLES20Instance( const char* appName, uint32_t appVersion )
	: Instance()
	, mImageReferences( decltype(mImageReferences)() )
{
	fDebug( appName, appVersion );

	EnumerateGpus();
}


int OpenGLES20Instance::EnumerateGpus()
{
	fDebug0();

	mGpuCount = 0;
	return mGpuCount;
}


Instance* OpenGLES20Instance::CreateDevice( int devid, int queueCount )
{
	OpenGLES20Instance* ret = (OpenGLES20Instance*)malloc( sizeof(OpenGLES20Instance) );
	memset( (void*)ret, 0, sizeof(OpenGLES20Instance) );
	memcpy( (void*)ret, (void*)this, sizeof(OpenGLES20Instance) );
	ret->mCpuRamCounter = 0;
	ret->mGpuRamCounter = 0;
	ret->mDevId = devid;

	fDebug( devid, queueCount );

	return ret;
}


uint64_t OpenGLES20Instance::ReferenceImage( Image* image )
{
	uint32_t glTextureID;
	glGenTextures( 1, &glTextureID );
	glBindTexture( GL_TEXTURE_2D, glTextureID );

	int textureWidth = geGetNextPower2( image->width() );
	int textureHeight = geGetNextPower2( image->height() );
	image->setMeta( "gles:width", textureWidth );
	image->setMeta( "gles:height", textureHeight );

	if ( textureWidth != (int)image->width() or textureHeight != (int)image->height() ) {
		uint32_t* data = (uint32_t*)Instance::baseInstance()->Malloc( textureWidth * textureHeight * sizeof( uint32_t ), false );
		for ( uint32_t y = 0; y < image->height(); y++ ) {
			memcpy( &data[ y * textureWidth ], &image->data()[ y * image->width() ], sizeof( uint32_t ) * image->width() );
		}
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
		Instance::baseInstance()->Free( data );
	} else {
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image->width(), image->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data() );
	}

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	((OpenGLES20Instance*)Instance::baseInstance())->AffectVRAM( image->width() * image->height() * sizeof( uint32_t ) );

	mImageReferencesMutex.lock();
	gDebug() << "New texture referenced ( " << glTextureID << " )\n";
	if ( mImageReferences.size() == 0 ) {
		mImageReferences.clear();
	}
	mImageReferences.insert( std::pair< uint64_t, Image* >( (uint64_t)glTextureID, image ) );
	mImageReferencesMutex.unlock();
	return glTextureID;
}


void OpenGLES20Instance::UnreferenceImage( uint64_t ref )
{
	if ( ref != 0 ) {
		uint32_t glTextureID = (uint32_t)ref;
		mImageReferencesMutex.lock();
		if ( mImageReferences.count( ref ) > 0 ) {
			Image* image = mImageReferences.at( ref );
			((OpenGLES20Instance*)Instance::baseInstance())->AffectVRAM( -1 * image->width() * image->height() * sizeof( uint32_t ) );
			mImageReferences.erase( ref );
		}
		mImageReferencesMutex.unlock();
		int width = 0, height = 0;
		glDeleteTextures( 1, &glTextureID );
	}
}


void OpenGLES20Instance::AffectVRAM( int64_t sz )
{
	mGpuRamCounter += sz;
}
