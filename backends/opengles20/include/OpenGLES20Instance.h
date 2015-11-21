/*
 * The GammaEngine Library 2.0 is a multiplatform OpenGLES20-based game engine
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

#ifndef OPENGLES20INSTANCE_H
#define OPENGLES20INSTANCE_H

#ifndef GE_WIN32
#define GL_GLEXT_PROTOTYPES
#endif

#ifdef GE_IOS
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif GE_WIN32
// #include <GL/gl.h>
// #include <GL/glext.h>
//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include <string.h>

// Windows tricks
#undef CreateWindow
#undef DrawText

#include "Instance.h"
#include "Image.h"
#include <map>
#include <string>
#include <mutex>

namespace GE {
	class Window;
	class Renderer;
	class Vertex;
	class Object;
// 	class Image;
}
using namespace GE;

class OpenGLES20Instance : public Instance
{
public:
	OpenGLES20Instance( const char* appName, uint32_t appVersion );
	virtual ~OpenGLES20Instance(){}

	virtual int EnumerateGpus();
	virtual Instance* CreateDevice( int devid, int queueCount = 1 );
	virtual uint64_t ReferenceImage( Image* image );
	virtual void UnreferenceImage( uint64_t ref );

	void AffectVRAM( int64_t sz );

private:
	std::map< uint64_t, Image* > mImageReferences;
	std::mutex mImageReferencesMutex;
};

#endif // OPENGLES20INSTANCE_H
