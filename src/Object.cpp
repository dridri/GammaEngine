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

#include <algorithm>

#include "Instance.h"
#include "Object.h"
#include "ObjectLoaderObj.h"
#include "File.h"
#include "Debug.h"


namespace GE {

std::vector< ObjectLoader* > Object::mObjectLoaders = std::vector< ObjectLoader* >();
static bool ObjectLoaderFirstCall = true;

Object::Object( Vertex* verts, uint32_t nVerts, uint32_t* indices, uint32_t nIndices )
	: mName( "" )
	, mVertices( verts )
	, mVerticesCount( nVerts )
	, mIndices( indices )
	, mIndicesCount( nIndices )
	, mMatrix( new Matrix() )
{
	mMatrices.insert( mMatrices.begin(), mMatrix );
}


Object::Object( const std::string filename, bool static_, Instance* instance )
	: mName( "" )
	, mVertices( nullptr )
	, mVerticesCount( 0 )
	, mIndices( nullptr )
	, mIndicesCount( 0 )
	, mMatrix( new Matrix() )
{
	if ( !instance ) {
		instance = Instance::baseInstance();
	}
	mInstance = instance;
	mMatrices.insert( mMatrices.begin(), mMatrix );

	File* file = new File( filename, File::READ );
	ObjectLoader* loader = GetLoader( filename, file );

	if ( loader ) {
		loader = loader->NewInstance();
		loader->Load( instance, file, static_ );

		mVertices = loader->mVertices;
		mIndices = loader->mIndices;
		mVerticesCount = loader->mVerticesCount;
		mIndicesCount = loader->mIndicesCount;

// 		delete loader;
		free( loader );
	}

	delete file;
}


Object* Object::Copy( bool copy_data )
{
	Object* ret = mInstance->CreateObject( mVertices, mVerticesCount, mIndices, mIndicesCount );
	ret->setName( mName );
	*ret->mMatrix = *mMatrix;
	for ( auto source = mMatrices.begin() + 1; source != mMatrices.end(); source++ ) {
		Matrix* dest = new Matrix();
		memcpy( dest->data(), (*source)->data(), sizeof(float) * 16 );
		ret->mMatrices.emplace_back( dest );
	}

	if ( copy_data ) {
		//TODO
	}
	return ret;
}


std::list< Object* > Object::LoadObjects( const std::string filename, bool static_, Instance* instance )
{
	if ( !instance ) {
		instance = Instance::baseInstance();
	}
	File* file = new File( filename, File::READ );
	ObjectLoader* loader = GetLoader( filename, file );
	std::list< Object* > ret;

	if ( loader ) {
		loader = loader->NewInstance();
		ret = loader->LoadObjects( instance, file, static_ );
// 		delete loader;
		free( loader );
	}

	delete file;
	return ret;
}


ObjectLoader* Object::GetLoader( const std::string filename, File* file )
{
	ObjectLoader* loader = nullptr;

	if ( ObjectLoaderFirstCall ) {
		AddObjectLoader( new ObjectLoaderObj() );
		ObjectLoaderFirstCall = false;
	}

	std::string extension = filename.substr( filename.rfind( "." ) + 1 );
	std::string first_line = file->ReadLine();
	std::transform( first_line.begin(), first_line.end(), first_line.begin(), ::tolower );
	file->Rewind();
	uint32_t file_magic = 0;
	file->Read( &file_magic, sizeof(file_magic) );
	file->Rewind();

	for ( size_t i = 0; i < mObjectLoaders.size(); i++ ) {
		if ( mObjectLoaders.at(i)->fileType() == ObjectLoader::BINARY ) {
			if ( mObjectLoaders.at(i)->magic() == file_magic ) {
				loader = mObjectLoaders.at(i);
				break;
			}
		} else {
			std::vector< std::string > patterns = mObjectLoaders.at(i)->contentPatterns();
			for ( size_t j = 0; j < patterns.size(); j++ ) {
				std::string test_case = patterns[j];
				std::transform( test_case.begin(), test_case.end(), test_case.begin(), ::tolower );
				if ( first_line.find( test_case ) ) {
					loader = mObjectLoaders.at(i);
				}
			}
		}
	}

	if ( !loader ) {
		for ( size_t i = 0; i < mObjectLoaders.size(); i++ ) {
			std::vector< std::string > extensions = mObjectLoaders.at(i)->extensions();
			for ( size_t j = 0; j < extensions.size(); j++ ) {
				std::string test_case = extensions[j];
				std::transform( test_case.begin(), test_case.end(), test_case.begin(), ::tolower );
				printf(" [%s].find(%s)\n", extension.c_str(), test_case.c_str());
				if ( extension.find( test_case ) == 0 ) {
					loader = mObjectLoaders.at(i);
					break;
				}
			}
		}
	}

	printf("  loader : %p\n", loader); fflush(stdout);
	return loader;
}


void Object::operator=( Object& other )
{
	mVertices = other.mVertices;
	mVerticesCount = other.mVerticesCount;
	mIndices = other.mIndices;
	mIndicesCount = other.mIndicesCount;
// 	memcpy( mMatrix->data(), other.mMatrix->data(), sizeof(float) * 16 );
	for ( auto source : other.mMatrices ) {
		Matrix* dest = new Matrix();
		memcpy( dest->data(), source->data(), sizeof(float) * 16 );
		mMatrices.emplace_back( dest );
	}
}


void Object::operator=( Object* other )
{
	mVertices = other->mVertices;
	mVerticesCount = other->mVerticesCount;
	mIndices = other->mIndices;
	mIndicesCount = other->mIndicesCount;
// 	memcpy( mMatrix->data(), other->mMatrix->data(), sizeof(float) * 16 );
	for ( auto source : other->mMatrices ) {
		Matrix* dest = new Matrix();
		memcpy( dest->data(), source->data(), sizeof(float) * 16 );
		mMatrices.emplace_back( dest );
	}
}


Object::~Object()
{
	delete mMatrix;
}


const std::string& Object::name() const
{
	return mName;
}


uint32_t Object::verticesCount() const
{
	return mVerticesCount;
}


uint32_t Object::indicesCount() const
{
	return mIndicesCount;
}


Vertex* Object::vertices() const
{
	return mVertices;
}


uint32_t* Object::indices() const
{
	return mIndices;
}


Matrix* Object::matrix( int instance ) const
{
	return mMatrices[ instance ];
}


Vector3f Object::position( int instance ) const
{
	return ( *mMatrices[ instance ] * Vector4f( 0.0f, 0.0f, 0.0f, 1.0f ) ).xyz();
// 	return Vector3f( mMatrices[ instance ]->m[12], mMatrices[ instance ]->m[13], mMatrices[ instance ]->m[14] );
}


int Object::instancesCount() const
{
	return mMatrices.size();
}


void Object::setName( const std::string& name )
{
	mName = name;
}


void Object::CreateInstances( int count )
{
	for ( int i = 0; i < count; i++ ) {
		Matrix* mat = new Matrix();
		memcpy( mat->data(), mMatrix->data(), sizeof(float) * 16 );
		mMatrices.emplace_back( mat );
	}
}


ObjectLoader* Object::AddObjectLoader( ObjectLoader* loader )
{
	mObjectLoaders.insert( mObjectLoaders.begin(), loader );
	return loader;
}


} // namespace GE
