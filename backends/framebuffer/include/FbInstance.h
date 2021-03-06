/*
 * The GammaEngine Library 2.0 is a multiplatform Fb-based game engine
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

#ifndef FRAMEBUFFERINSTANCE_H
#define FRAMEBUFFERINSTANCE_H

#include <string.h>

#include "Instance.h"
#include "Image.h"
#include "FbWindow.h"
#include <map>
#include <string>
#include <mutex>

namespace GE {
	class Renderer;
	class Vertex;
	class Object;
// 	class Image;
}
using namespace GE;

class FbInstance : public Instance
{
public:
	FbInstance( void* pBackend, const char* appName, uint32_t appVersion );
	virtual ~FbInstance(){}

	virtual int EnumerateGpus();
	virtual Instance* CreateDevice( int devid, int queueCount = 1 );
	virtual uint64_t ReferenceImage( Image* image );
	virtual void UnreferenceImage( uint64_t ref );
	virtual void UpdateImageData( Image* image, uint64_t ref );

	void AffectVRAM( int64_t sz );

	FbWindow* boundWindow() const;
	void BindWindow( FbWindow* win );

private:
	FbWindow* mBoundWindow;
};

#endif // FRAMEBUFFERINSTANCE_H
