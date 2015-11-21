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
#include <unordered_map>
#include "MeshBuilder.h"
#include "Instance.h"
#include "Debug.h"

using namespace GE;

MeshBuilder::MeshBuilder( BaseType basetype, const Vector3f& size, int tesslevel )
	: mBaseType( basetype )
	, mSize( size )
{
	if ( basetype == Plane ) {
		const Vector3f plane[4] = {
			{ -0.5f * size.x, -0.5f * size.y, 0.0f },
			{  0.5f * size.x, -0.5f * size.y, 0.0f },
			{  0.5f * size.x,  0.5f * size.y, 0.0f },
			{ -0.5f * size.x,  0.5f * size.y, 0.0f },
		};
		mFaces.emplace_back( Face( plane[0], plane[1], plane[2] ) );
		mFaces.emplace_back( Face( plane[0], plane[2], plane[3] ) );
		for ( int i = 0; i < tesslevel; i++ ) {
			Tesselate( None );
		}
	}
	if ( basetype == Sphere ) {
		const Vector3f icosahedron[12] = {
			{  1.618033, 1.0, 0.0 }, // 0
			{ -1.618033, 1.0, 0.0 }, // 1
			{  1.618033,-1.0, 0.0 }, // 2
			{ -1.618033,-1.0, 0.0 }, // 3
			{  1.0, 0.0, 1.618033 }, // 4
			{  1.0, 0.0,-1.618033 }, // 5
			{ -1.0, 0.0, 1.618033 }, // 6
			{ -1.0, 0.0,-1.618033 }, // 7
			{  0.0, 1.618033, 1.0 }, // 8
			{  0.0,-1.618033, 1.0 }, // 9
			{  0.0, 1.618033,-1.0 }, // 10
			{  0.0,-1.618033,-1.0 }  // 11
		};
		mFaces.emplace_back( Face( icosahedron[0], icosahedron[8], icosahedron[4] ) );
		mFaces.emplace_back( Face( icosahedron[1], icosahedron[10], icosahedron[7] ) );
		mFaces.emplace_back( Face( icosahedron[2], icosahedron[9], icosahedron[11] ) );
		mFaces.emplace_back( Face( icosahedron[7], icosahedron[3], icosahedron[1] ) );    
		mFaces.emplace_back( Face( icosahedron[0], icosahedron[5], icosahedron[10] ) );
		mFaces.emplace_back( Face( icosahedron[3], icosahedron[9], icosahedron[6] ) );
		mFaces.emplace_back( Face( icosahedron[3], icosahedron[11], icosahedron[9] ) );
		mFaces.emplace_back( Face( icosahedron[8], icosahedron[6], icosahedron[4] ) );    
		mFaces.emplace_back( Face( icosahedron[2], icosahedron[4], icosahedron[9] ) );
		mFaces.emplace_back( Face( icosahedron[3], icosahedron[7], icosahedron[11] ) );
		mFaces.emplace_back( Face( icosahedron[4], icosahedron[2], icosahedron[0] ) );
		mFaces.emplace_back( Face( icosahedron[9], icosahedron[4], icosahedron[6] ) );    
		mFaces.emplace_back( Face( icosahedron[2], icosahedron[11], icosahedron[5] ) );
		mFaces.emplace_back( Face( icosahedron[0], icosahedron[10], icosahedron[8] ) );
		mFaces.emplace_back( Face( icosahedron[5], icosahedron[0], icosahedron[2] ) );
		mFaces.emplace_back( Face( icosahedron[10], icosahedron[5], icosahedron[7] ) );    
		mFaces.emplace_back( Face( icosahedron[1], icosahedron[6], icosahedron[8] ) );
		mFaces.emplace_back( Face( icosahedron[1], icosahedron[8], icosahedron[10] ) );
		mFaces.emplace_back( Face( icosahedron[6], icosahedron[1], icosahedron[3] ) );
		mFaces.emplace_back( Face( icosahedron[11], icosahedron[7], icosahedron[5] ) );
		for ( size_t i = 0; i < mFaces.size(); i++ ) {
			Vector3f p0 = mFaces[i].p0();
			Vector3f p1 = mFaces[i].p1();
			Vector3f p2 = mFaces[i].p2();
			p0.x *= size.x;
			p0.y *= size.y;
			p0.z *= size.z;
			p1.x *= size.x;
			p1.y *= size.y;
			p1.z *= size.z;
			p2.x *= size.x;
			p2.y *= size.y;
			p2.z *= size.z;
			mFaces[i] = Face( p0, p1, p2 );
		}
		for ( int i = 0; i < tesslevel; i++ ) {
			Tesselate( Normalize );
		}
	}
}

MeshBuilder::~MeshBuilder()
{
}


void MeshBuilder::Tesselate( MeshBuilder::TesselationMethod method )
{
	std::vector< Face > newFaces;

	/*    0
	     /\
	  01/__\02
	   /\  /\
	  /__\/__\
	 1   12   2
	*/

	for ( size_t i = 0; i < mFaces.size(); i++ ) {
		Vector3f p0 = mFaces[i].p0();
		Vector3f p1 = mFaces[i].p1();
		Vector3f p2 = mFaces[i].p2();
		Vector3f p01 = ( p0 + p1 ) * 0.5f;
		Vector3f p12 = ( p1 + p2 ) * 0.5f;
		Vector3f p02 = ( p0 + p2 ) * 0.5f;
		if ( method == Normalize ) {
			p01.normalize();
			p12.normalize();
			p02.normalize();
			p01.x *= mSize.x;
			p01.y *= mSize.y;
			p01.z *= mSize.z;
			p12.x *= mSize.x;
			p12.y *= mSize.y;
			p12.z *= mSize.z;
			p02.x *= mSize.x;
			p02.y *= mSize.y;
			p02.z *= mSize.z;
		}
		newFaces.emplace_back( Face( p0, p01, p02 ) );
		newFaces.emplace_back( Face( p01, p1, p12 ) );
		newFaces.emplace_back( Face( p01, p12, p02 ) );
		newFaces.emplace_back( Face( p02, p12, p2 ) );
	}

	mFaces = newFaces;
}


void MeshBuilder::Translate( const Vector3f& vec )
{
	for ( size_t i = 0; i < mFaces.size(); i++ ) {
		Vector3f p0 = mFaces[i].p0() + vec;
		Vector3f p1 = mFaces[i].p1() + vec;
		Vector3f p2 = mFaces[i].p2() + vec;
		mFaces[i] = Face( p0, p1, p2 );
	}
}


void MeshBuilder::RemoveFaces( MeshBuilderRemoveCb cb, void* cbdata )
{
	for ( size_t i = 0; i < mFaces.size(); i++ ) {
		if ( !cb( cbdata, &mFaces[i] ) ) {
			mFaces.erase( mFaces.begin() + i );
			--i;
		}
	}
}


void MeshBuilder::SinglePassFaces( MeshBuilder::MeshBuilderPassCb cb, void* cbdata )
{
	for ( size_t i = 0; i < mFaces.size(); i++ ) {
		cb( cbdata, &mFaces[i] );
	}
}


void MeshBuilder::GenerateVertexArray( Instance* instance, Vertex** verts, uint32_t* nVerts )
{
	Vertex* ret = (Vertex*)instance->Malloc( sizeof(Vertex) * mFaces.size() * 3 );

	for ( size_t i = 0; i < mFaces.size(); i++ ) {
		ret[i * 3 + 0] = Vertex( mFaces[i].p0() );
		ret[i * 3 + 1] = Vertex( mFaces[i].p1() );
		ret[i * 3 + 2] = Vertex( mFaces[i].p2() );
	}

	*verts = ret;
	*nVerts = mFaces.size() * 3;
}


std::size_t MeshBuilder::VertexHasher::operator()( const Vertex& k ) const
{
	using std::size_t;
	using std::hash;
	using std::string;

	std::size_t a = ((hash<float>()(k.x) ^ (hash<float>()(k.y) << 1)) >> 1) ^ ((hash<float>()(k.z) ^ (hash<int>()(k.weight) << 1)) >> 1);
	std::size_t b = ((hash<float>()(k.color[0]) ^ (hash<float>()(k.color[1]) << 1)) >> 1) ^ ((hash<float>()(k.color[2]) ^ (hash<int>()(k.color[3]) << 1)) >> 1);
	std::size_t c = ((hash<float>()(k.u) ^ (hash<float>()(k.v) << 1)) >> 1) ^ ((hash<float>()(k.w) ^ (hash<int>()(k._align1) << 1)) >> 1);
	std::size_t d = ((hash<float>()(k.nx) ^ (hash<float>()(k.ny) << 1)) >> 1) ^ ((hash<float>()(k.nz) ^ (hash<int>()(k._align2) << 1)) >> 1);
	return a ^ ((b << 1) >> 1) ^ ((c << 1) >> 2) ^ (d << 1);
}


std::size_t MeshBuilder::VertexHasher::operator()( const Vertex* k ) const
{
	using std::size_t;
	using std::hash;
	using std::string;

	std::size_t a = ((hash<float>()(k->x) ^ (hash<float>()(k->y) << 1)) >> 1) ^ ((hash<float>()(k->z) ^ (hash<int>()(k->weight) << 1)) >> 1);
	std::size_t b = ((hash<float>()(k->color[0]) ^ (hash<float>()(k->color[1]) << 1)) >> 1) ^ ((hash<float>()(k->color[2]) ^ (hash<int>()(k->color[3]) << 1)) >> 1);
	std::size_t c = ((hash<float>()(k->u) ^ (hash<float>()(k->v) << 1)) >> 1) ^ ((hash<float>()(k->w) ^ (hash<int>()(k->_align1) << 1)) >> 1);
	std::size_t d = ((hash<float>()(k->nx) ^ (hash<float>()(k->ny) << 1)) >> 1) ^ ((hash<float>()(k->nz) ^ (hash<int>()(k->_align2) << 1)) >> 1);
	return a ^ ((b << 1) >> 1) ^ ((c << 1) >> 2) ^ (d << 1);
}


void MeshBuilder::GenerateIndexedVertexArray( Instance* instance, Vertex** pVertices, uint32_t* nVerts, uint32_t** pIndices, uint32_t* nIndices, bool invert_faces )
{

	std::vector< Vertex > vertices;
	std::vector< uint32_t > indices;
	std::unordered_map< Vertex, uint32_t, VertexHasher > elements;

	for ( size_t i = 0; i < mFaces.size(); i++ )
	{
		Vector3f normal = Vector3f();
		if ( mBaseType == Plane ) {
			normal = ( mFaces[i].p1() - mFaces[i].p0() ) ^ ( mFaces[i].p2() - mFaces[i].p0() );
		}
		normal.normalize();
		if ( invert_faces ) {
			normal = -normal;
		}

		int start = invert_faces ? 2 : 0;
		int end = invert_faces ? 0 : 2;
		int incr = invert_faces ? -1 : 1;

		for ( int j = start; j != end + incr; j += incr ) {
			Vertex vertex = Vertex( mFaces[i].p( j ) );
			vertex.nx = normal.x;
			vertex.ny = normal.y;
			vertex.nz = normal.z;

			uint32_t idx = 0;
			if ( elements.find( vertex ) != elements.end() ) {
				idx = elements[ vertex ];
			} else {
				idx = vertices.size();
				vertices.emplace_back( vertex );
				elements.insert( std::make_pair( vertex, idx ) );
			}
			indices.emplace_back( idx );
		}
	}

/*
	std::vector< Vertex > vertices;
	std::vector< uint32_t > indices;

	for ( size_t i = 0; i < mFaces.size(); i++ ) {
		Vector3f normal = Vector3f();
		if ( mBaseType == Plane ) {
			normal = ( mFaces[i].p1() - mFaces[i].p0() ) ^ ( mFaces[i].p2() - mFaces[i].p0() );
		}
		normal.normalize();
		if ( invert_faces ) {
			normal = -normal;
		}
		for ( size_t j = 0; j < 3; j++ ) {
			Vertex vertex = Vertex( mFaces[i].p( invert_faces * ( 2 - j ) + !invert_faces * j ) );
			vertex.nx = normal.x;
			vertex.ny = normal.y;
			vertex.nz = normal.z;

			int64_t idx = -1;
			for ( size_t k = 0; k < vertices.size(); k++ ) {
				if ( vertices[k] == vertex ) {
					idx = k;
					break;
				}
			}
			if ( idx < 0 ) {
				idx = vertices.size();
				vertices.emplace_back( vertex );
			}
			indices.emplace_back( idx );
		}
	}
*/
	*pVertices = (Vertex*)instance->Malloc( sizeof(Vertex) * vertices.size() );
	*pIndices = (uint32_t*)instance->Malloc( sizeof(uint32_t) * indices.size() );
	memcpy( *pVertices, vertices.data(), sizeof(Vertex) * vertices.size() );
	memcpy( *pIndices, indices.data(), sizeof(uint32_t) * indices.size() );
	*nVerts = vertices.size();
	*nIndices = indices.size();
}
