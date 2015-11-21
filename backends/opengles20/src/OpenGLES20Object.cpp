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
#include "OpenGLES20Object.h"
#include "File.h"
#include "Image.h"
#include "Debug.h"

// #include <algorithm>


using namespace GE;

extern "C" GE::Object* CreateObject( Vertex* verts, uint32_t nVerts, uint32_t* indices, uint32_t nIndices ) {
	return new OpenGLES20Object( verts, nVerts, indices, nIndices );
}

extern "C" GE::Object* LoadObject( const std::string filename, Instance* instance ) {
	return new OpenGLES20Object( filename, instance );
}


OpenGLES20Object::OpenGLES20Object( Vertex* verts, uint32_t nVerts, uint32_t* indices, uint32_t nIndices )
	: Object( verts, nVerts, indices, nIndices )
{
}


OpenGLES20Object::OpenGLES20Object( const std::string filename, Instance* instance )
	: Object( filename, instance )
{
}


OpenGLES20Object::~OpenGLES20Object()
{
// 	Object::~Object();
}


const std::vector< std::pair< Image*, uint32_t > >* OpenGLES20Object::textures( Instance* instance )
{
	if ( mTextures.find( instance ) != mTextures.end() ) {
		return &mTextures[ instance ];
	}
	return nullptr;
}


void OpenGLES20Object::setTexture( Instance* instance, int unit, Image* texture )
{
	std::vector< std::pair< Image*, uint32_t > >* textures = nullptr;
	if ( mTextures.find( instance ) == mTextures.end() ) {
		std::vector< std::pair< Image*, uint32_t > > empty;
		mTextures.insert( std::make_pair( instance, empty ) );
	}
	textures = &mTextures[ instance ];

	if ( (int)textures->size() <= unit ) {
		textures->resize( unit + 1, std::make_pair( nullptr, 0 ) );
	}

	(*textures)[unit] = std::make_pair( texture, texture->serverReference( instance ) );
}


void OpenGLES20Object::UpdateVertices( Instance* instance, Vertex* verts, uint32_t offset, uint32_t count )
{
}


void OpenGLES20Object::UploadMatrix( Instance* instance )
{
}
