/*
 * The GammaEngine Library 2.0 is a multiplatform OpenGL43-based game engine
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

#ifndef VULKANINSTANCE_H
#define VULKANINSTANCE_H

#ifndef _WIN32
#define GL_GLEXT_PROTOTYPES
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#include <string.h>

// Windows tricks
#undef CreateWindow

#include <string>
#include "Instance.h"

namespace GE {
	class Window;
	class Renderer;
	class Vertex;
	class Object;
}
using namespace GE;

class OpenGL43Instance : public Instance
{
public:
	OpenGL43Instance( const char* appName, uint32_t appVersion );
	virtual ~OpenGL43Instance(){}

	virtual int EnumerateGpus();
	virtual Instance* CreateDevice( int devid, int queueCount = 1 );
	virtual uint64_t ReferenceImage( Image* image );
	virtual void UnreferenceImage( uint64_t ref );

	void AffectVRAM( int64_t sz );
private:
};

#endif // VULKANINSTANCE_H
