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

#include "OpenGL43Instance.h"
#include "Vertex.h"
#include "Image.h"
#include "Debug.h"

#ifndef ALIGN
	#define ALIGN(x, align) (((x)+((align)-1))&~((align)-1))
#endif

extern "C" GE::Instance* CreateInstance( const char* appName, uint32_t appVersion ) {
	return new OpenGL43Instance( appName, appVersion );
}


OpenGL43Instance::OpenGL43Instance( const char* appName, uint32_t appVersion )
	: Instance()
{
	fDebug( appName, appVersion );

	EnumerateGpus();
}


int OpenGL43Instance::EnumerateGpus()
{
	fDebug0();

	mGpuCount = 0;
	return mGpuCount;
}


Instance* OpenGL43Instance::CreateDevice( int devid, int queueCount )
{
	OpenGL43Instance* ret = (OpenGL43Instance*)malloc( sizeof(OpenGL43Instance) );
	memset( ret, 0, sizeof(OpenGL43Instance) );
	memcpy( ret, this, sizeof(OpenGL43Instance) );
	ret->mCpuRamCounter = 0;
	ret->mGpuRamCounter = 0;
	ret->mDevId = devid;

	fDebug( devid, queueCount );

	return ret;
}


uint64_t OpenGL43Instance::ReferenceImage( Image* image )
{
	uint32_t glTextureID;
	glGenTextures( 1, &glTextureID );
	glBindTexture( GL_TEXTURE_2D, glTextureID );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, image->width(), image->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data() );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( image->width() * image->height() * sizeof( uint32_t ) );

	gDebug() << "New texture referenced ( VRAM usage : " << ( ((OpenGL43Instance*)Instance::baseInstance())->mGpuRamCounter / 1024 ) << " KB )\n";
	return glTextureID;
}


void OpenGL43Instance::UnreferenceImage( uint64_t ref )
{
	if ( ref != 0 ) {
		uint32_t glTextureID = (uint32_t)ref;
		int width = 0, height = 0;
		glBindTexture( GL_TEXTURE_2D, glTextureID );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );
		((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( -1 * width * height * sizeof( uint32_t ) );
		glDeleteTextures( 1, &glTextureID );
	}
}


void OpenGL43Instance::AffectVRAM( int64_t sz )
{
	mGpuRamCounter += sz;
}
