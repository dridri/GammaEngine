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

#include <string.h>
#include "Vertex.h"

#define COLORf(r, g, b, a) ( ( ( (uint32_t)( a * 255.0f ) & 0xFF ) << 24 ) | ( ( (uint32_t)( b * 255.0f ) & 0xFF ) << 16 ) | ( ( (uint32_t)( g * 255.0f ) & 0xFF ) << 8 ) | ( ( (uint32_t)( r * 255.0f ) & 0xFF ) ) )

namespace GE {

Vertex::Vertex( const Vector3f& pos, const Vector4f& color, const Vector3f& normal, const Vector3f& texcoords )
	: u( texcoords.x ), v( texcoords.y ), w( texcoords.z ), _align1( 0.0f )
	, color{ color.x, color.y, color.z, color.w }
	, nx( normal.x ), ny( normal.y ), nz( normal.z ), _align2( 0.0f )
	, x( pos.x ), y( pos.y ), z( pos.z ), weight( 0.0f )
{
}


bool Vertex::operator==( const Vertex& other ) const
{
	return memcmp( this, &other, sizeof( Vertex ) ) == 0;
}

Vertex2D::Vertex2D( const Vector2f& pos, const Vector4f& color, const Vector2f& texcoords )
	: color( COLORf( color.x, color.y, color.z, color.w ) )
	, u( texcoords.x ), v( texcoords.y )
	, x( pos.x ), y( pos.y )
{
}


bool Vertex2D::operator==( const Vertex2D& other ) const
{
	return memcmp( this, &other, sizeof( Vertex2D ) ) == 0;
}


} // namespace GE
