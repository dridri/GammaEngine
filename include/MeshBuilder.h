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

#ifndef GE_MESHBUILDER_H
#define GE_MESHBUILDER_H

#include <vector>
#include <functional>
#include "Vertex.h"
#include "Vector.h"

namespace GE
{

class Instance;

class MeshBuilder
{
public:
	class Face;
	typedef bool (*MeshBuilderRemoveCb)( void*, Face* );
	typedef void (*MeshBuilderPassCb)( void*, Face* );

	typedef enum {
		Plane,
		Disc,
		Cube,
		Sphere,
		Cylinder,
		Cone,
	} BaseType;
	typedef enum {
		None,
		Normalize,
		NormalizeH,
		NormalizeV,
	} TesselationMethod;

	MeshBuilder( BaseType basetype, const Vector3f& size = Vector3f(1,1,1), int tesslevel = 0 );
	~MeshBuilder();

	std::vector< Face >& faces();

	void Tesselate( TesselationMethod method );
	void Translate( const Vector3f& vec );
	void RemoveFaces( MeshBuilderRemoveCb cb, void* cbdata = nullptr );
	void SinglePassFaces( MeshBuilderPassCb cb, void* cbdata = nullptr );
	void RemoveFaces( std::function<bool(Face*)> cb );
	void SinglePassFaces( std::function<void(Face*)> cb );

	void GenerateVertexArray( Instance* instance, Vertex** verts, uint32_t* nVerts );
	void GenerateIndexedVertexArray( Instance* instance, Vertex** verts, uint32_t* nVerts, uint32_t** indices, uint32_t* nIndices, bool invert_faces = false );

	class Face
	{
	public:
		Face( const Vector3f& p0, const Vector3f& p1, const Vector3f& p2 ) : mp0(p0), mp1(p1), mp2(p2) {}
		const Vector3f& p(uint32_t i) { return (i == 0) ? mp0 : ( (i == 1) ? mp1 : mp2 ); }
		const Vector3f& p0() { return mp0; }
		const Vector3f& p1() { return mp1; }
		const Vector3f& p2() { return mp2; }
	protected:
		Vector3f mp0, mp1, mp2;
	};

	struct VertexHasher
	{
		std::size_t operator()(const Vertex& k) const;
		std::size_t operator()(const Vertex* k) const;
	};

protected:
	BaseType mBaseType;
	Vector3f mSize;
	std::vector< Face > mFaces;
};
}

#endif // GE_MESHBUILDER_H
