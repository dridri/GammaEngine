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

#ifndef OPENGLES20OBJECT_H
#define OPENGLES20OBJECT_H

#include <string>
#include <vector>
#include <map>

#include "Object.h"
#include "Vertex.h"
#include "Matrix.h"

using namespace GE;

class OpenGLES20Object : public Object
{
public:
	OpenGLES20Object( Vertex* verts = nullptr, uint32_t nVerts = 0, uint32_t* indices = nullptr, uint32_t nIndices = 0 );
	OpenGLES20Object( const std::string filename, Instance* instance = nullptr );
	~OpenGLES20Object();

	virtual void setTexture( Instance* instance, int unit, Image* texture );
	virtual void UpdateVertices( Instance* instance, Vertex* verts, uint32_t offset, uint32_t count );
	virtual void UploadMatrix( Instance* instance );

	const std::vector< std::pair< Image*, uint32_t > >* textures( Instance* instance );

protected:
	std::map< Instance*, std::vector< std::pair< Image*, uint32_t > > > mTextures;
};


#endif // OPENGLES20OBJECT_H
