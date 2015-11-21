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

#ifndef VERTEX_H
#define VERTEX_H

#include <stdint.h>
#include "Vector.h"

namespace GE {

class Vertex
{
public:
	Vertex( const Vector3f& pos = Vector3f(), const Vector4f& color = Vector4f(1,1,1,1), const Vector3f& normal = Vector3f(), const Vector3f& texcoords = Vector3f() );

	bool operator==( const Vertex& other ) const;

// Attributes defined as public for fast access
public:
	float u, v, w, _align1;
	float color[4];
	float nx, ny, nz, _align2;
	float x, y, z, weight;
} __attribute__((packed, aligned(16))); // Stay cool with memory

class Vertex2D
{
public:
	Vertex2D( const Vector2f& pos = Vector2f(), const Vector4f& color = Vector4f(1,1,1,1), const Vector2f& texcoords = Vector2f() );

	bool operator==( const Vertex2D& other ) const;

// Attributes defined as public for fast access
public:
	uint32_t color;
	float u, v;
	float x, y;
} __attribute__((packed)); // Stay cool with memory

/*
typedef struct Vertex {
	float u, v, w, _align1;
	float color[4];
	float nx, ny, nz, _align2;
	float x, y, z, weight;
} Vertex;
*/
} // namespace GE

#endif // VERTEX_H
