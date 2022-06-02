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

#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <vector>
#include <list>
#include <map>

#include "Vertex.h"
#include "Matrix.h"
#include "MetaObject.h"

namespace GE {

class Instance;
class Renderer;
class ObjectLoader;
class File;
class Image;

class Object : public MetaObject
{
public:
	Object( VertexBase* verts = nullptr, uint32_t nVerts = 0, uint32_t* indices = nullptr, uint32_t nIndices = 0 );
	Object( const std::string filename, bool static_ = false, Instance* instance = nullptr );
	virtual ~Object();

	const std::string& name() const;
	uint32_t verticesCount() const;
	uint32_t indicesCount() const;
	uint32_t indicesRenderCount() const;
	VertexBase* vertices() const;
	uint32_t* indices() const;
	Matrix* matrix( int instance = 0 ) const;
	Vector3f position( int instance = 0 ) const;
	int instancesCount() const;

	void setName( const std::string& name );
	void CreateInstances( int count );
	void RemoveInstance( uint32_t idx );

	void setIndicesRenderCount( uint32_t count );

	virtual void setTexture( Instance* instance, int unit, Image* texture ) = 0;
	virtual void ReuploadVertices( Renderer* renderer, uint32_t offset, uint32_t count ) = 0;
	virtual void UpdateVertices( Instance* instance, VertexBase* verts, uint32_t offset, uint32_t count ) = 0;
	virtual void UploadMatrix( Instance* instance ) = 0;

	Object* Copy( bool copy_data = false );
	void operator=( Object& other );
	void operator=( Object* other );

	static std::list< Object* > LoadObjects( const std::string filename, bool static_ = false, Instance* instance = nullptr );
	static ObjectLoader* AddObjectLoader( ObjectLoader* loader );

protected:
	static ObjectLoader* GetLoader( const std::string filename, File* file );
	Instance* mInstance;
	std::string mName;
	VertexBase* mVertices;
	uint32_t mVerticesCount;
	uint32_t* mIndices;
	uint32_t mIndicesCount;
	uint32_t mIndicesRenderCount;
	Matrix* mMatrix;

	std::vector< Matrix* > mMatrices;

	static std::vector< ObjectLoader* > mObjectLoaders;
};


class ObjectLoader : public Object
{
public:
	typedef enum {
		UNKNOWN = -1,
		BINARY,
		TEXT
	} TYPE;
	ObjectLoader() : Object() { ; }
	virtual ~ObjectLoader() { ; }
	virtual TYPE fileType() = 0;
	virtual uint32_t magic() = 0;
	virtual std::vector< std::string > contentPatterns() = 0;
	virtual std::vector< std::string > extensions() = 0;
	virtual ObjectLoader* NewInstance() = 0;
	virtual void Load( Instance* instance, File* file, bool static_ = false ) = 0;
	virtual std::list< Object* > LoadObjects( Instance* instance, File* file, bool static_ = false ) = 0;

	virtual void setTexture( Instance* instance, int unit, Image* texture ){}
	virtual void ReuploadVertices( Renderer* renderer, uint32_t offset, uint32_t count ){};
	virtual void UpdateVertices( Instance* instance, VertexBase* verts, uint32_t offset, uint32_t count ){}
	virtual void UploadMatrix( Instance* instance ){}
};


} // namespace GE

#endif // OBJECT_H
