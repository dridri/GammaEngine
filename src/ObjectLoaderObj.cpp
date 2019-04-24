/*
 * <one line to give the library's name and an idea of what it does.>
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

#include <unordered_map>

#include "ObjectLoaderObj.h"
#include "File.h"
#include "Vector.h"
#include "Instance.h"
#include "Image.h"
#include "Debug.h"

#include <stdlib.h>

#define RGBA(r,g,b,a) (((uint8_t)(a) << 24)|((uint8_t)(b) << 16)|((uint8_t)(g) << 8)|(uint8_t)(r))
#define RGBAf(r,g,b,a) RGBA(((r)*255.0f),((g)*255.0f),((b)*255.0f),((a)*255.0f))

namespace GE {


ObjectLoaderObj::ObjectLoaderObj()
	: ObjectLoader()
{
}


ObjectLoaderObj::TYPE ObjectLoaderObj::fileType()
{
	return TEXT;
}


uint32_t ObjectLoaderObj::magic()
{
	return 0;
}


std::vector< std::string > ObjectLoaderObj::contentPatterns()
{
	std::vector< std::string > ret;
	ret.emplace_back( "obj" );
	ret.emplace_back( "wavefront" );
	return ret;
}


std::vector< std::string > ObjectLoaderObj::extensions()
{
	std::vector< std::string > ret;
	ret.emplace_back( "obj" );
	return ret;
}


ObjectLoader* ObjectLoaderObj::NewInstance()
{
	return new ObjectLoaderObj();
}


std::list< Object* > ObjectLoaderObj::LoadObjects( Instance* instance, File* file, bool static_ )
{
	fDebug( instance, file );

	std::list< Object* > objects;
	std::string currentName = "";

	std::string line;
	std::stringstream tokenizer;
	std::stringstream tokenizer2;
	std::string type;
	std::string str;
	std::string str2;
	float f = 0.0f;

	std::vector< Vector3f > verts;
	std::vector< Vector3f > tex;
	std::vector< Vector3f > norms;
	Material base_mat;
	base_mat.diffuse[0] = base_mat.diffuse[1] = base_mat.diffuse[2] = base_mat.diffuse[3]= 1.0f;
	Material* mat = &base_mat;

	std::map< Material*, int32_t > currentMaterials; // material, texidx or -1
	uint32_t curr_tex_id = 0;

	uint32_t iVert = 0;
	uint32_t iTex = 0;
	uint32_t iNorm = 0;
	uint32_t iFace = 0;

	verts.reserve( 1024 );
	tex.reserve( 1024 );
	norms.reserve( 1024 );

	std::unordered_map< std::string, uint32_t > elements;

	Vertex* buff = ( Vertex* )instance->Malloc( sizeof( Vertex ) * 1024 * 1024 );
	uint32_t iBuff = 0;
	uint32_t maxBuff = 1024 * 1024;

	uint32_t* indices = ( uint32_t* )instance->Malloc( sizeof( uint32_t ) * 128 * 3 * 1024 );
	uint32_t iIndices = 0;
	uint32_t maxIndices = 128 * 3 * 1024;

	while ( file->ReadLine( line ) )
	{
		if ( line[0] == '#' ) {
			continue;
		}

		tokenizer.clear();
		tokenizer.str( line );

		type = "";
		tokenizer >> type;

		if ( type == "mtllib" ) {
			str = "";
			tokenizer >> str;
			LoadMaterials( instance, file, str );
		}

		if ( type == "g" ) {
			// Insert loaded data
			if ( elements.size() > 0 ) {
				gDebug() << "Inserting object " << currentName << "\n";
				uint32_t finalVerticesCount = iBuff;
				Vertex* finalVertices = ( Vertex* )instance->Malloc( sizeof( Vertex ) * iBuff, false );
				memcpy( finalVertices, buff, sizeof( Vertex ) * iBuff );
				uint32_t finalIndicesCount = iIndices;
				uint32_t* finalIndices = ( uint32_t* )instance->Malloc( sizeof( uint32_t ) * iIndices, false );
				memcpy( finalIndices, indices, sizeof( uint32_t ) * iIndices );
				Vector3f center = Vector3f();
				if ( !static_ ) {
					for ( uint32_t i = 0; i < iBuff; i++ ) {
						center = center + Vector3f( finalVertices[i].x, finalVertices[i].y, finalVertices[i].z ) * ( 1.0f / (float)iBuff );
					}
					for ( uint32_t i = 0; i < iBuff; i++ ) {
						finalVertices[i].x -= center.x;
						finalVertices[i].y -= center.y;
						finalVertices[i].z -= center.z;
					}
				}
				Object* obj = instance->CreateObject( finalVertices, finalVerticesCount, finalIndices, finalIndicesCount );
				obj->matrix()->Translate( center.x, center.y, center.z );
				obj->setName( currentName );
				for ( std::pair< Material*, int32_t > mat : currentMaterials ) {
					if ( mat.second >= 0 ) {
						if ( mat.first->map_Kd ) {
							obj->setTexture( instance, mat.second, mat.first->map_Kd );
						}
						if ( mat.first->map_bump ) {
							obj->setTexture( instance, mat.second + 1, mat.first->map_bump );
						}
					}
				}
/*
				if ( mat && mat != &base_mat ) {
					if ( mat->map_Kd ) obj->setTexture( instance, 0, mat->map_Kd );
// 					if ( mat->map_bump ) obj->setTexture( instance, 1, mat->map_bump );
				}
*/
				objects.emplace_back( obj );
				currentMaterials.clear();
				curr_tex_id = 0;
			}

			elements.clear();
			iBuff = 0;
			iIndices = 0;
			currentName = "";
			tokenizer >> currentName;
		}

		if ( type == "usemtl" ) {
			tokenizer >> str;
			if ( mMaterials.find( str ) != mMaterials.end() ) {
				mat = mMaterials[ str ];
			} else {
				mat = &base_mat;
			}
			if ( currentMaterials.find( mat ) == currentMaterials.end() ) {
				currentMaterials.emplace( std::make_pair( mat, mat->map_Kd ? curr_tex_id++ : -1 ) );
				if ( mat->map_bump ) {
					curr_tex_id++;
				}
			}
		}

		if ( type == "v" ) {
			if ( iVert + 1 >= verts.size() ) {
				verts.resize( verts.size() + 1024 );
			}
			tokenizer >> f; verts[iVert].x = f;
			tokenizer >> f; verts[iVert].y = f;
			tokenizer >> f; verts[iVert].z = f;
			iVert++;
		}

		if ( type == "vn" ) {
			if ( iNorm + 1 >= norms.size() ) {
				norms.resize( norms.size() + 1024 );
			}
			tokenizer >> f; norms[iNorm].x = f;
			tokenizer >> f; norms[iNorm].y = f;
			tokenizer >> f; norms[iNorm].z = f;
			iNorm++;
		}

		if ( type == "vt" ) {
			if ( iTex + 1 >= tex.size() ) {
				tex.resize( tex.size() + 1024 );
			}
			tokenizer >> f; tex[iTex].x = f;
			tokenizer >> f; tex[iTex].y = f;
			tokenizer >> f; tex[iTex].z = f;
			iTex++;
		}

		if ( type == "f" )
		{
			uint32_t face_indices[4] = { 0 }; // Indice in GE's object
			uint32_t n = 0;
			while ( !tokenizer.eof() )
			{
				uint32_t idx = 0;
				str = "";
				tokenizer >> str;
				if ( str == "" || str.length() <= 1 ) {
					break;
				}
				if ( elements.find( str ) != elements.end() )
				{
					idx = elements[ str ];
				}
				else
				{
					idx = iBuff;
					elements.insert( std::pair< std::string, uint32_t >( str, idx ) );
					int point_indexes[3] = { 0 }; // Indexes in OBJ vertices tables
					uint32_t p = 0;
					tokenizer2.clear();
					tokenizer2.str( str );
					while(std::getline( tokenizer2, str2, '/') ) {
						if ( str2.length() <= 0 ) {
							point_indexes[p] = -1;
						} else {
							point_indexes[p] = atoi( str2.data() ) - 1;
						}
// 						printf("  point_indexes[%d] = %d\n", p, point_indexes[p]);
						p++;
					}
					if ( p == 3 ) {
						Vector3f ppos;
						Vector4f ptex;
						Vector3f pnorm;
						if ( point_indexes[0] >= 0 ) {
							ppos = verts[point_indexes[0]];
						}
						if ( point_indexes[2] >= 0 ) {
							pnorm = norms[point_indexes[2]];
						}
						if ( point_indexes[1] >= 0 ) {
							ptex = tex[point_indexes[1]];
						}
						ptex.w = currentMaterials[mat];
						if ( ptex.w > 0 ) {
							gDebug() << ptex.w;
						}
						buff[idx] = Vertex( ppos, Vector4f(mat->diffuse), pnorm, ptex );
						buff[idx].v = -buff[idx].v;
					}

					if ( iBuff + 1 >= maxBuff ) {
						buff = ( Vertex* )instance->Realloc( buff, sizeof( Vertex ) * ( maxBuff + 1024 ) );
						maxBuff += 1024;
					}
					iBuff++;
				}
				face_indices[n] = idx;
				n++;
			}
			if ( n == 3 ) { // Triangle
				if ( iIndices + 3 >= maxIndices ) {
					indices = ( uint32_t* )instance->Realloc( indices, sizeof( uint32_t ) * ( maxIndices + 128 * 3 ) );
					maxIndices += 128 * 3;
				}
				memcpy( &indices[iIndices], face_indices, sizeof( uint32_t ) * 3 );
				iIndices += 3;
			} else if ( n == 4 ) { // Quad
				// TODO
				iIndices += 4;
			}
			iFace++;
		}
	}

	uint32_t finalVerticesCount = iBuff;
	Vertex* finalVertices = ( Vertex* )instance->Malloc( sizeof( Vertex ) * iBuff, false );
	memcpy( finalVertices, buff, sizeof( Vertex ) * iBuff );
	uint32_t finalIndicesCount = iIndices;
	uint32_t* finalIndices = ( uint32_t* )instance->Malloc( sizeof( uint32_t ) * iIndices, false );
	memcpy( finalIndices, indices, sizeof( uint32_t ) * iIndices );
	Vector3f center = Vector3f();
	if ( !static_ ) {
		for ( uint32_t i = 0; i < iBuff; i++ ) {
			center = center + Vector3f( finalVertices[i].x, finalVertices[i].y, finalVertices[i].z ) * ( 1.0f / (float)iBuff );
		}
		for ( uint32_t i = 0; i < iBuff; i++ ) {
			finalVertices[i].x -= center.x;
			finalVertices[i].y -= center.y;
			finalVertices[i].z -= center.z;
		}
	}
	Object* obj = instance->CreateObject( finalVertices, finalVerticesCount, finalIndices, finalIndicesCount );
	obj->matrix()->Translate( center.x, center.y, center.z );
	obj->setName( currentName );
	for ( std::pair< Material*, int32_t > mat : currentMaterials ) {
		if ( mat.second >= 0 and mat.first->map_Kd ) {
			obj->setTexture( instance, mat.second, mat.first->map_Kd );
		}
	}
	objects.emplace_back( obj );

	for ( std::map< std::string, Material* >::iterator it = mMaterials.begin(); it != mMaterials.end(); ++it ) {
		delete (*it).second;
	}
	instance->Free( buff );
	instance->Free( indices );

	return objects;
}


void ObjectLoaderObj::Load( Instance* instance, File* file, bool static_ )
{
	fDebug( instance, file );

	std::string line;
	std::stringstream tokenizer;
	std::stringstream tokenizer2;
	std::string type;
	std::string str;
	std::string str2;
	float f = 0.0f;

	std::vector< Vector3f > verts;
	std::vector< Vector3f > tex;
	std::vector< Vector3f > norms;
	Material base_mat;
	base_mat.diffuse[0] = base_mat.diffuse[1] = base_mat.diffuse[2] = base_mat.diffuse[3] = 1.0f;
	Material* mat = &base_mat;

	uint32_t iVert = 0;
	uint32_t iTex = 0;
	uint32_t iNorm = 0;
	uint32_t iFace = 0;

	verts.reserve( 1024 );
	tex.reserve( 1024 );
	norms.reserve( 1024 );

	std::unordered_map< std::string, uint32_t > elements;

	Vertex* buff = ( Vertex* )instance->Malloc( sizeof( Vertex ) * 1024 * 2048 );
	uint32_t iBuff = 0;
	uint32_t maxBuff = 1024 * 2048;

	uint32_t* indices = ( uint32_t* )instance->Malloc( sizeof( uint32_t ) * 128 * 3 * 2048 );
	uint32_t iIndices = 0;
	uint32_t maxIndices = 128 * 3 * 2048;

// 	uint32_t ln = 0;

	while ( file->ReadLine( line ) )
	{
		if ( line[0] == '#' ) {
			continue;
		}
		tokenizer.clear();
		tokenizer.str( line );

		type = "";
		tokenizer >> type;

		if ( type == "mtllib" ) {
			tokenizer >> str;
			LoadMaterials( instance, file, str );
		}

		if ( type == "usemtl" ) {
			tokenizer >> str;
			if ( mMaterials.find( str ) != mMaterials.end() ) {
				mat = mMaterials[ str ];
			} else {
				mat = &base_mat;
			}
		}

		if ( type == "v" ) {
			if ( iVert + 1 >= verts.size() ) {
				verts.resize( verts.size() + 1024 );
			}
			tokenizer >> f; verts[iVert].x = f;
			tokenizer >> f; verts[iVert].y = f;
			tokenizer >> f; verts[iVert].z = f;
			iVert++;
		}

		if ( type == "vn" ) {
			if ( iNorm + 1 >= norms.size() ) {
				norms.resize( norms.size() + 1024 );
			}
			tokenizer >> f; norms[iNorm].x = f;
			tokenizer >> f; norms[iNorm].y = f;
			tokenizer >> f; norms[iNorm].z = f;
			iNorm++;
		}

		if ( type == "vt" ) {
			if ( iTex + 1 >= tex.size() ) {
				tex.resize( tex.size() + 1024 );
			}
			tokenizer >> f; tex[iTex].x = f;
			tokenizer >> f; tex[iTex].y = f;
			tokenizer >> f; tex[iTex].z = f;
			iTex++;
		}

		if ( type == "f" )
		{
			uint32_t face_indices[4] = { 0 }; // Indice in GE's object
			uint32_t n = 0;
			while ( !tokenizer.eof() )
			{
				uint32_t idx = 0;
				str = "";
				tokenizer >> str;
				if ( str == "" || str.length() <= 1 ) {
					break;
				}
				if ( elements.find( str ) != elements.end() )
				{
					idx = elements[ str ];
				}
				else
				{
					idx = iBuff;
					elements.insert( std::pair< std::string, uint32_t >( str, idx ) );
					int point_indexes[3] = { 0 }; // Indexes in OBJ vertices tables
					uint32_t p = 0;
					tokenizer2.clear();
					tokenizer2.str( str );
					while(std::getline( tokenizer2, str2, '/') ) {
						point_indexes[p] = atoi( str2.data() ) - 1;
// 						printf("  point_indexes[%d] = %d\n", p, point_indexes[p]);
						p++;
					}
					if ( p == 3 ) {
						buff[idx] = Vertex( verts[point_indexes[0]], mat->diffuse, norms[point_indexes[2]], tex[point_indexes[1]] );
						buff[idx].v = - buff[idx].v;
					}

					if ( iBuff + 1 >= maxBuff ) {
						buff = ( Vertex* )instance->Realloc( buff, sizeof( Vertex ) * ( maxBuff + 1024 ) );
						maxBuff += 1024;
					}
					iBuff++;
				}
				face_indices[n] = idx;
				n++;
			}
			if ( n == 3 ) { // Triangle
				if ( iIndices + 3 >= maxIndices ) {
					indices = ( uint32_t* )instance->Realloc( indices, sizeof( uint32_t ) * ( maxIndices + 128 * 3 ) );
					maxIndices += 128 * 3;
				}
				memcpy( &indices[iIndices], face_indices, sizeof( uint32_t ) * 3 );
				iIndices += 3;
			} else if ( n == 4 ) { // Quad
				// TODO
				iIndices += 4;
			}
			iFace++;
		}
	}

	for ( std::map< std::string, Material* >::iterator it = mMaterials.begin(); it != mMaterials.end(); ++it ) {
		delete (*it).second;
	}

	mVerticesCount = iBuff;
	mVertices = ( Vertex* )instance->Malloc( sizeof( Vertex ) * iBuff, false );
	memcpy( mVertices, buff, sizeof( Vertex ) * iBuff );
	instance->Free( buff );

	mIndicesCount = iIndices;
	mIndices = ( uint32_t* )instance->Malloc( sizeof( uint32_t ) * iIndices, false );
	memcpy( mIndices, indices, sizeof( uint32_t ) * iIndices );
	instance->Free( indices );
/*
	if ( mat && mat != &base_mat ) {
		if ( mat->map_Kd ) setTexture( instance, 0, mat->map_Kd );
		if ( mat->map_bump ) setTexture( instance, 1, mat->map_bump );
	}
*/
}


void ObjectLoaderObj::LoadMaterials( Instance* instance, File* base_file, std::string filename )
{
	File* file = new File( base_file, filename, File::READ );

	std::string line;
	std::stringstream tokenizer;
	std::string type;
	std::string str;
	std::map < std::string, Image* > textures;
	float alpha = 1.0f;
	float f1 = 0.0f;
	float f2 = 0.0f;
	float f3 = 0.0f;
	Material* mat = nullptr;

	while ( file->ReadLine( line ) )
	{
		if ( line[0] == '#' ) {
			continue;
		}
		tokenizer.clear();
		tokenizer.str( line );

		type = "";
		tokenizer >> type;

		if ( type == "newmtl" ) {
			tokenizer >> str;
			mat = new Material;
			memset( mat, 0, sizeof(Material) );
			mMaterials.insert( std::pair< std::string, Material* >( str, mat ) );
			alpha = 1.0f;
		}

		if ( type == "d" ) {
			tokenizer >> alpha;
		}

		if ( type == "Ka" ) {
			tokenizer >> f1;
			tokenizer >> f2;
			tokenizer >> f3;
// 			mat->ambient = RGBAf( f1, f2, f3, alpha );
			mat->ambient[0] = f1;
			mat->ambient[1] = f2;
			mat->ambient[2] = f3;
			mat->ambient[3] = alpha;
		}

		if ( type == "Kd" ) {
			tokenizer >> f1;
			tokenizer >> f2;
			tokenizer >> f3;
// 			mat->diffuse = RGBAf( f1, f2, f3, alpha );
			mat->diffuse[0] = f1;
			mat->diffuse[1] = f2;
			mat->diffuse[2] = f3;
			mat->diffuse[3] = alpha;
		}

		if ( type == "Ks" ) {
			tokenizer >> f1;
			tokenizer >> f2;
			tokenizer >> f3;
// 			mat->specular = RGBAf( f1, f2, f3, alpha );
			mat->specular[0] = f1;
			mat->specular[1] = f2;
			mat->specular[2] = f3;
			mat->specular[3] = alpha;
		}

		if ( type == "map_Kd" ) {
			tokenizer >> str;
			gDebug() << "texfile : " << str << "\n";
			if ( textures.count( str ) > 0 ) {
				mat->map_Kd = textures.at( str );
			} else {
				File* texfile = new File( file, str, File::READ );
				mat->map_Kd = new Image( texfile, str.substr( str.rfind( "." ) + 1 ), instance );
				textures.emplace( str, mat->map_Kd );
				delete texfile;
			}
		}

		if ( type == "map_bump" || type == "bump" ) {
			// TODO : handle -bm option
			tokenizer >> str;
			gDebug() << "bump map file : " << str << "\n";
			File* texfile = new File( file, str, File::READ );
			mat->map_bump = new Image( texfile, str.substr( str.rfind( "." ) + 1 ), instance );
			mat->map_bump->setType( Image::ImageNorm );
			delete texfile;
		}
	}

	delete file;
	gDebug() << "Textures count : " << textures.size() << "\n";
	gDebug() << "Materials count : " << mMaterials.size() << "\n";
}

} // namespace GE
