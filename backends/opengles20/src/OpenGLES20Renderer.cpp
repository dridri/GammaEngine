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

#include <fstream>
#include <vector>

#include "Instance.h"
#include "OpenGLES20Object.h"
#include "OpenGLES20Instance.h"
#include "OpenGLES20Renderer.h"
#include "OpenGLES20Window.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"

#if ( ( defined( GL_EXT_draw_instanced ) || defined( glDrawElementsInstancedEXT ) ) && !defined( GE_ANDROID ) )
	#define DRAW_INSTANCED_AVAILABLE
#else
	#warning "GLES2 instancing NOT available !"
#endif

#if ( ( defined( GL_EXT_base_instance ) || defined( glDrawArraysInstancedBaseInstanceEXT ) ) && !defined( GE_ANDROID ) )
	#define DRAW_ARRAYS_BASE_INSTANCE_AVAILABLE
#else
	#warning "GLES2 instancing NOT available !"
#endif

extern "C" GE::Renderer* CreateRenderer( GE::Instance* instance ) {
	return new OpenGLES20Renderer( instance );
}

static const char* vertex_shader_include =
#if ( !defined( GE_ANDROID ) && !defined( GE_IOS ) && !defined( GE_EGL ) )
	"#version 140\n"
#endif
#if ( defined( GE_ANDROID ) || defined( GE_IOS ) || defined( GE_EGL ) )
	"#define in attribute\n"
	"#define out varying\n"
// 	"#define flat\n"
#endif
	"precision highp float;\n"
	"#define geTexture2D(x) ge_Texture0\n"
	"#define geTexture3D(x) ge_Texture0\n"
	"\n"
#ifdef GE_IOS
	"#define GE_IOS\n"
#endif
#ifdef DRAW_INSTANCED_AVAILABLE
	#ifdef GE_IOS
		"#define ge_InstanceID gl_InstanceIDEXT\n"
	#else
		"#define ge_InstanceID gl_InstanceID\n"
	#endif
#else
	"uniform int ge_InstanceID;\n"
#endif
	"in vec4 ge_VertexTexcoord;\n"
	"in vec4 ge_VertexColor;\n"
	"in vec4 ge_VertexNormal;\n"
	"in vec4 ge_VertexPosition;\n"
	"\n"
	"uniform mat4 ge_ObjectMatrix;\n"
	"uniform mat4 ge_ProjectionMatrix;\n"
	"uniform mat4 ge_ViewMatrix;\n"
	"uniform sampler2D ge_Texture0;\n"
	"\n"
	"#define ge_Position gl_Position\n"
;

static const char* fragment_shader_include =
#if ( !defined( GE_ANDROID ) && !defined( GE_IOS ) && !defined( GE_EGL ) )
	"#version 130\n"
#endif
	"precision highp float;\n"
	"uniform sampler2D ge_Texture0;\n"
#if ( defined( GE_ANDROID ) || defined( GE_IOS ) || defined( GE_EGL ) )
	"#define in varying\n"
	"#define flat\n"
	"#define texture texture2D\n"
#endif
#if ( defined( GE_ANDROID ) || defined( GE_IOS ) || defined( GE_EGL ) )
	"#define ge_FragColor gl_FragColor\n"
#else
	"out vec4 ge_FragColor;\n"
#endif
;

bool OpenGLES20Renderer::s2DActive = false;
uint32_t OpenGLES20Renderer::mLastVertexDefinitionHash = 0;

OpenGLES20Renderer::OpenGLES20Renderer( Instance* instance )
	: mReady( false )
	, mInstance( instance ? instance : Instance::baseInstance() )
	, mMatrixObjects( 0 )
	, mMatrixObjectsSize( 0 )
	, mVertexDefinition( Vertex::vertexDefinition() )
	, mRenderMode( GL_TRIANGLES )
	, mDepthTestEnabled( false )
	, mBlendingEnabled( false )
	, mBlendingSrc( GL_SRC_ALPHA )
	, mBlendingDst( GL_ONE_MINUS_SRC_ALPHA )
	, mShader( 0 )
	, mVertexShader( 0 )
	, mFragmentShader( 0 )
	, mMatrixProjectionID( 0 )
	, mMatrixViewID( 0 )
	, mMatrixObjectID( 0 )
{

	mMatrixProjection = new Matrix();
	mMatrixProjection->Perspective( 60.0f, 16.0f / 9.0f, 0.01f, 1000.0f );
	mMatrixView = new Matrix();
	mMatrixView->Identity();
}


OpenGLES20Renderer::~OpenGLES20Renderer()
{
}


Matrix* OpenGLES20Renderer::projectionMatrix()
{
	return mMatrixProjection;
}


int OpenGLES20Renderer::LoadVertexShader( const void* data, size_t size )
{
	mReady = false;
	if ( mVertexShader ) {
		glDeleteShader( mVertexShader );
	}
	mVertexShader = glCreateShader( GL_VERTEX_SHADER );

#ifdef GE_IOS
	char* fulldata = (char*)malloc( strlen((char*)data) + strlen(vertex_shader_include) + 2 );
	sprintf( fulldata, "%s%s", vertex_shader_include, (char*)data );
	glShaderSource( mVertexShader, 1, (const char**)&fulldata, NULL );
	free( fulldata );
#else
	const char* array[] = { vertex_shader_include, (char*)data };
	glShaderSource( mVertexShader, sizeof(array)/sizeof(char*), array, NULL );
#endif
	glCompileShader( mVertexShader );
	char log[4096] = "";
	int logsize = 4096;
	glGetShaderInfoLog( mVertexShader, logsize, &logsize, log );
	gDebug() << "vertex compile : " << log << "\n";

	return 0;
}


int OpenGLES20Renderer::LoadGeometryShader( const void* data, size_t size )
{
	gDebug() << "Warning : Geometry shader not supported by this backend !\n";
	return 0;
}


int OpenGLES20Renderer::LoadFragmentShader( const void* data, size_t size )
{
	mReady = false;
	if ( mFragmentShader ) {
		glDeleteShader( mFragmentShader );
	}

	mFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

#ifdef GE_IOS
	char* fulldata = (char*)malloc( strlen((char*)data) + strlen(fragment_shader_include) + 2 );
	sprintf( fulldata, "%s%s", fragment_shader_include, (char*)data );
	glShaderSource( mFragmentShader, 1, (const char**)&fulldata, NULL );
	free( fulldata );
#else
	const char* array[] = { fragment_shader_include, (char*)data };
	glShaderSource( mFragmentShader, sizeof(array)/sizeof(char*), array, NULL );
#endif
	glCompileShader( mFragmentShader );
	char log[4096] = "";
	int logsize = 4096;
	glGetShaderInfoLog( mFragmentShader, logsize, &logsize, log );
	gDebug() << "fragment compile : " << log << "\n";

	return 0;
}


int OpenGLES20Renderer::LoadVertexShader( const std::string& file )
{
	mReady = false;

	size_t sz = 0;
	uint8_t* data = loadShader( file, &sz );
	int ret = LoadVertexShader( data, sz );
	mInstance->Free( data );

	return ret;
}


int OpenGLES20Renderer::LoadGeometryShader( const std::string& file )
{
	mReady = false;

	size_t sz = 0;
	uint8_t* data = loadShader( file, &sz );
	int ret = LoadGeometryShader( data, sz );
	mInstance->Free( data );

	return ret;
}


int OpenGLES20Renderer::LoadFragmentShader( const std::string& file )
{
	mReady = false;

	size_t sz = 0;
	uint8_t* data = loadShader( file, &sz );
	int ret = LoadFragmentShader( data, sz );
	mInstance->Free( data );

	return ret;
}


void OpenGLES20Renderer::setVertexDefinition( const VertexDefinition& vertexDefinition )
{
	mVertexDefinition = vertexDefinition;
}


void OpenGLES20Renderer::setRenderMode( const RenderMode& mode )
{
	switch ( mode ) {
		case Points : { mRenderMode = GL_POINTS; break; }
		case Lines : { mRenderMode = GL_LINES; break; }
		case LineStrip : { mRenderMode = GL_LINE_STRIP; break; }
		case TriangleStrip : { mRenderMode = GL_TRIANGLE_STRIP; break; }
		case Triangles : default : { mRenderMode = GL_TRIANGLES; break; }
	}
}


void OpenGLES20Renderer::setDepthTestEnabled( bool en )
{
	mDepthTestEnabled = en;
}


void OpenGLES20Renderer::setBlendingEnabled( bool en )
{
	mBlendingEnabled = en;
}


void OpenGLES20Renderer::setBlendingMode( BlendingMode source, BlendingMode dest )
{
	auto set = []( BlendingMode m, int32_t def ) {
		switch ( m ) {
			case Zero:
				return GL_ZERO;
			case One:
				return GL_ONE;
			case SrcColor:
				return GL_SRC_COLOR;
			case OneMinusSrcColor:
				return GL_ONE_MINUS_SRC_COLOR;
			case DstColor:
				return GL_DST_COLOR;
			case OneMinusDstColor:
				return GL_ONE_MINUS_DST_COLOR;
			case SrcAlpha:
				return GL_SRC_ALPHA;
			case OneMinusSrcAlpha:
				return GL_ONE_MINUS_SRC_ALPHA;
			case DstAlpha:
				return GL_DST_ALPHA;
			case OneMinusDstAlpha:
				return GL_ONE_MINUS_DST_ALPHA;
			default:
				break;
		}
		return def;
	};

	mBlendingSrc = set( source, GL_SRC_COLOR );
	mBlendingSrc = set( dest, GL_DST_COLOR );
}


void OpenGLES20Renderer::createPipeline()
{
	if ( mShader ) {
		glDeleteProgram( mShader );
	}
	mShader = glCreateProgram();
	glAttachShader( mShader, mVertexShader );
	glAttachShader( mShader, mFragmentShader );

// 	glBindAttribLocation( mShader, 0, "ge_VertexTexcoord" );
// 	glBindAttribLocation( mShader, 1, "ge_VertexColor" );
// 	glBindAttribLocation( mShader, 2, "ge_VertexNormal" );
// 	glBindAttribLocation( mShader, 3, "ge_VertexPosition" );

	glLinkProgram( mShader );

	mVertexTexcoordID = glGetAttribLocation( mShader, "ge_VertexTexcoord" );
	mVertexColorID = glGetAttribLocation( mShader, "ge_VertexColor" );
	mVertexNormalID = glGetAttribLocation( mShader, "ge_VertexNormal" );
	mVertexPositionID = glGetAttribLocation( mShader, "ge_VertexPosition" );
	mVertexDefinition = VertexDefinition( sizeof(Vertex) );
	mVertexDefinition.addAttribute( mVertexTexcoordID, 4, VertexDefinition::Float32, sizeof(Vertex), 0 );
	mVertexDefinition.addAttribute( mVertexColorID, 4, VertexDefinition::Float32, sizeof(Vertex), sizeof( float ) * 4 );
	mVertexDefinition.addAttribute( mVertexNormalID, 4, VertexDefinition::Float32, sizeof(Vertex), sizeof( float ) * 4 * 2 );
	mVertexDefinition.addAttribute( mVertexPositionID, 4, VertexDefinition::Float32, sizeof(Vertex), sizeof( float ) * 4 * 3 );

	char log[4096] = "";
	int logsize = 4096;
	glGetProgramInfoLog( mShader, logsize, &logsize, log );
	gDebug() << "program compile : " << log << "\n";

	mMatrixProjectionID = glGetUniformLocation( mShader, "ge_ProjectionMatrix" );
	mMatrixViewID = glGetUniformLocation( mShader, "ge_ViewMatrix" );
	mMatrixObjectID = glGetUniformLocation( mShader, "ge_ObjectMatrix" );

	mFloatTimeID = glGetUniformLocation( mShader, "ge_Time" );
#if !( defined( DRAW_INSTANCED_AVAILABLE ) )
	mIntInstanceID = glGetUniformLocation( mShader, "ge_InstanceID" );
#endif

// 	glEnableVertexAttribArray( mVertexTexcoordID );
// 	glEnableVertexAttribArray( mVertexColorID );
// 	glEnableVertexAttribArray( mVertexNormalID );
// 	glEnableVertexAttribArray( mVertexPositionID );

	mReady = true;
}


void OpenGLES20Renderer::AddObject( Object* obj )
{
	mObjects.emplace_back( (Object*)obj );
/*
	bool opaque = true;
	Vertex* vertices = obj->vertices();
	for ( uint32_t i = 0; i < obj->verticesCount(); i++ ) {
		if ( vertices[i].color[3] < 1.0 ) {
			opaque = false;
			break;
		}
	}

	if ( opaque ) {
		mObjects.insert( mObjects.begin(), obj );
	} else {
		mObjects.emplace_back( (Object*)obj );
	}
*/
}


void OpenGLES20Renderer::AddLight( Light* light )
{
	mLights.emplace_back( light );
}


void OpenGLES20Renderer::VertexPoolAppend( VertexBase** pVertices, uint32_t& pVerticesPoolSize, uint32_t& pVerticesCount, VertexBase* append, uint32_t count )
{
	if ( pVerticesCount + count > pVerticesPoolSize ) {
		pVerticesPoolSize += ( ( count + 8192 ) / 8192 * 8192 );
		VertexBase* vertices = (VertexBase*)mInstance->Malloc( mVertexDefinition.size() * pVerticesPoolSize, false );
		if ( *pVertices ) {
			memcpy( vertices, *pVertices, mVertexDefinition.size() * pVerticesCount );
			mInstance->Free( *pVertices );
		}
		*pVertices = vertices;
	}

	memcpy( &((uint8_t*)*pVertices)[ mVertexDefinition.size() * pVerticesCount ], append, mVertexDefinition.size() * count );

	pVerticesCount += count;
}


void OpenGLES20Renderer::Compute()
{
	if ( !mReady ) {
		createPipeline();
	}

	VertexBase* vertices = nullptr;
	std::vector< uint32_t > indices;
	uint32_t verticesPoolSize = 0;
	uint32_t verticesCount = 0;

	if ( mMatrixObjects ) {
		for ( size_t i = 0; i < mObjects.size(); i++ ) {
			float* ptr = mObjects[i]->matrix()->m;
			if ( ptr >= mMatrixObjects and ptr < &mMatrixObjects[mMatrixObjectsSize] ) {
				mObjects[i]->matrix()->setDataPointer( nullptr );
			}
		}
		mInstance->Free( mMatrixObjects );
	}
	mMatrixObjects = (float*)mInstance->Malloc( sizeof(float) * 16 * mObjects.size() );
	mMatrixObjectsSize = 16 * mObjects.size();

	mObjectsVerticesStart.clear();

	uint32_t baseVertex = 0;
	for ( size_t i = 0; i < mObjects.size(); i++ ) {
		gDebug() << "Object[" << i << "] baseVertex = " << baseVertex << ( i > 0 ? ( "(" + std::to_string( mObjects[i-1]->verticesCount() ) + ")" ) : "" ) << "\n";
		mObjects[i]->matrix()->setDataPointer( &mMatrixObjects[ 16 * i ] );
		for ( uint32_t j = 0; j < mObjects[i]->indicesCount(); j++ ) {
// 			indices.emplace_back( mObjects[i]->indices()[j] );
			indices.emplace_back( baseVertex + mObjects[i]->indices()[j] );
		}
// 		indices.insert( indices.end(), mObjects[i]->indices(), &mObjects[i]->indices()[mObjects[i]->indicesCount()] );
		mObjectsVerticesStart.emplace( std::make_pair( mObjects[i], baseVertex ) );
		VertexPoolAppend( &vertices, verticesPoolSize, verticesCount, mObjects[i]->vertices(), mObjects[i]->verticesCount() );
		baseVertex += mObjects[i]->verticesCount();
	}

	glGenBuffers( 1, &mIBO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW );
	((OpenGLES20Instance*)Instance::baseInstance())->AffectVRAM( sizeof(uint32_t) * indices.size() );

	glGenBuffers( 1, &mVBO );
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferData( GL_ARRAY_BUFFER, verticesCount * mVertexDefinition.size(), vertices, GL_STATIC_DRAW );
	((OpenGLES20Instance*)Instance::baseInstance())->AffectVRAM( sizeof(Vertex) * verticesCount );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


void OpenGLES20Renderer::UpdateVertexArray( VertexBase* data, uint32_t offset, uint32_t count )
{
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
// 	glBufferSubData( GL_ARRAY_BUFFER, offset * sizeof(Vertex), count * sizeof(Vertex), &data[offset] ); //TODO : use size of vertexDefinition
	glBufferData( GL_ARRAY_BUFFER, count * mVertexDefinition.size(), data, GL_STATIC_DRAW );
	((OpenGLES20Instance*)Instance::baseInstance())->AffectVRAM( sizeof(Vertex) * count );
}


void OpenGLES20Renderer::UpdateIndicesArray( uint32_t* data, uint32_t offset, uint32_t count )
{
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW );
	((OpenGLES20Instance*)Instance::baseInstance())->AffectVRAM( sizeof(uint32_t) * count );
}


void OpenGLES20Renderer::Draw()
{
/*
	if ( !mReady ) {
		Compute();
	}
*/
	if ( s2DActive ) {
		glEnable( GL_DEPTH_TEST );
		s2DActive = false;
	}

	if ( mBlendingEnabled ) {
		glEnable( GL_BLEND );
		glBlendFunc( mBlendingSrc, mBlendingDst );
	} else {
		glDisable( GL_BLEND );
	}
	if ( mDepthTestEnabled ) {
		glEnable( GL_DEPTH_TEST );
	} else {
		glDisable( GL_DEPTH_TEST );
	}

	glUseProgram( mShader );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIBO );
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );

	glUniform1f( mFloatTimeID, Time::GetSeconds() );
	glUniformMatrix4fv( mMatrixProjectionID, 1, GL_FALSE, mMatrixProjection->data() );
	glUniformMatrix4fv( mMatrixViewID, 1, GL_FALSE, mMatrixView->data() );

/*
	glEnableVertexAttribArray( mVertexTexcoordID );
	glVertexAttribPointer( mVertexTexcoordID, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( 0 ) );
	glEnableVertexAttribArray( mVertexColorID );
	glVertexAttribPointer( mVertexColorID, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 ) );
	glEnableVertexAttribArray( mVertexNormalID );
	glVertexAttribPointer( mVertexNormalID, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 + sizeof( float ) * 4 ) );
	glEnableVertexAttribArray( mVertexPositionID );
	glVertexAttribPointer( mVertexPositionID, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 + sizeof( float ) * 4 + sizeof( float ) * 4 ) );
*/
// 	if ( mVertexDefinition.hash() != mLastVertexDefinitionHash ) {
		for ( const VertexDefinition::Attribute& attrib : mVertexDefinition.attributes() ) {
			GLenum type = 0;
			switch( attrib.type ) {
				case VertexDefinition::UInt8 : { type = GL_UNSIGNED_BYTE; break; }
				case VertexDefinition::Int8 : { type = GL_BYTE; break; }
				case VertexDefinition::UInt16 : { type = GL_UNSIGNED_SHORT; break; }
				case VertexDefinition::Int16 : { type = GL_SHORT; break; }
				case VertexDefinition::UInt32 : { type = GL_UNSIGNED_INT; break; }
				case VertexDefinition::Int32 : { type = GL_INT; break; }
	// 			case VertexDefinition::Float16 : { type = GL_HALF_FLOAT; break; }
				case VertexDefinition::Float32 : { type = GL_FLOAT; break; }
	// 			case VertexDefinition::Float64 : { type = GL_DOUBLE; break; }
				default: { gDebug() << "WARNING : Unsupported vertex attribute type " << attrib.type; break; }
			}
			glEnableVertexAttribArray( attrib.attributeID );
			glVertexAttribPointer( attrib.attributeID, attrib.count, type, attrib.normalize ? GL_TRUE : GL_FALSE, attrib.stride, (void*)(uintptr_t)attrib.offset );
		}
		mLastVertexDefinitionHash = mVertexDefinition.hash();
// 	}

	uint32_t iIndices = 0;
	for ( size_t i = 0; i < mObjects.size(); i++ ) {
		const std::vector< std::pair< Image*, uint32_t > >* textures = dynamic_cast<OpenGLES20Object*>(mObjects[i])->textures( mInstance );
		if ( textures and textures->size() > 0 ) {
			glBindTexture( GL_TEXTURE_2D, textures->at(0).second );
		}
		glUniformMatrix4fv( mMatrixObjectID, 1, GL_FALSE, mObjects[i]->matrix()->data() );
#ifdef DRAW_INSTANCED_AVAILABLE
		glDrawElementsInstancedEXT( mRenderMode, mObjects[i]->indicesCount(), GL_UNSIGNED_INT, (void*)(uint64_t)( iIndices * sizeof(uint32_t) ), mObjects[i]->instancesCount() );
#else
		for ( uint32_t j = 0; j < (uint32_t)mObjects[i]->instancesCount(); j++ ) {
			glUniform1i( mIntInstanceID, j );
			glDrawElements( mRenderMode, mObjects[i]->indicesCount(), GL_UNSIGNED_INT, (void*)(uint64_t)( iIndices * sizeof(uint32_t) ) );
		}
#endif
		iIndices += mObjects[i]->indicesCount();
	}

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glUseProgram( 0 );
}


void OpenGLES20Renderer::Draw( uint32_t inddicesOffset, uint32_t indicesCount, uint32_t verticesOffset, uint32_t verticesCount, uint32_t instanceCount, uint32_t baseInstance )
{
	if ( s2DActive ) {
		glEnable( GL_DEPTH_TEST );
		s2DActive = false;
	}

	if ( mBlendingEnabled ) {
		glEnable( GL_BLEND );
		glBlendFunc( mBlendingSrc, mBlendingDst );
	} else {
		glDisable( GL_BLEND );
	}
	if ( mDepthTestEnabled ) {
		glEnable( GL_DEPTH_TEST );
	} else {
		glDisable( GL_DEPTH_TEST );
	}

	glUseProgram( mShader );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIBO );
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );

	glUniform1f( mFloatTimeID, Time::GetSeconds() );
	glUniformMatrix4fv( mMatrixProjectionID, 1, GL_FALSE, mMatrixProjection->data() );
	glUniformMatrix4fv( mMatrixViewID, 1, GL_FALSE, mMatrixView->data() );
/*
	glEnableVertexAttribArray( mVertexTexcoordID );
	glVertexAttribPointer( mVertexTexcoordID, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( 0 ) );
	glEnableVertexAttribArray( mVertexColorID );
	glVertexAttribPointer( mVertexColorID, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 ) );
	glEnableVertexAttribArray( mVertexNormalID );
	glVertexAttribPointer( mVertexNormalID, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 + sizeof( float ) * 4 ) );
	glEnableVertexAttribArray( mVertexPositionID );
	glVertexAttribPointer( mVertexPositionID, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 + sizeof( float ) * 4 + sizeof( float ) * 4 ) );
*/
// 	if ( mVertexDefinition.hash() != mLastVertexDefinitionHash ) {
		for ( const VertexDefinition::Attribute& attrib : mVertexDefinition.attributes() ) {
			GLenum type = 0;
			switch( attrib.type ) {
				case VertexDefinition::UInt8 : { type = GL_UNSIGNED_BYTE; break; }
				case VertexDefinition::Int8 : { type = GL_BYTE; break; }
				case VertexDefinition::UInt16 : { type = GL_UNSIGNED_SHORT; break; }
				case VertexDefinition::Int16 : { type = GL_SHORT; break; }
				case VertexDefinition::UInt32 : { type = GL_UNSIGNED_INT; break; }
				case VertexDefinition::Int32 : { type = GL_INT; break; }
	// 			case VertexDefinition::Float16 : { type = GL_HALF_FLOAT; break; }
				case VertexDefinition::Float32 : { type = GL_FLOAT; break; }
	// 			case VertexDefinition::Float64 : { type = GL_DOUBLE; break; }
				default: { gDebug() << "WARNING : Unsupported vertex attribute type " << attrib.type; break; }
			}
			glEnableVertexAttribArray( attrib.attributeID );
			glVertexAttribPointer( attrib.attributeID, attrib.count, type, attrib.normalize ? GL_TRUE : GL_FALSE, attrib.stride, (void*)(uintptr_t)attrib.offset );
		}
		mLastVertexDefinitionHash = mVertexDefinition.hash();
// 	}

	if ( indicesCount > 0 ) {
#ifdef DRAW_INSTANCED_AVAILABLE
		glDrawElementsInstancedEXT( mRenderMode, indicesCount, GL_UNSIGNED_INT, (void*)(uint64_t)( inddicesOffset * sizeof(uint32_t) ), instanceCount );
#else
		for ( uint32_t j = 0; j < (uint32_t)instanceCount; j++ ) {
			glUniform1i( mIntInstanceID, baseInstance + j );
			glDrawElements( mRenderMode, indicesCount, GL_UNSIGNED_INT, (void*)(uint64_t)( inddicesOffset * sizeof(uint32_t) ) );
		}
#endif
	} else {
#ifdef DRAW_ARRAYS_BASE_INSTANCE_AVAILABLE
		glDrawArraysInstancedBaseInstanceEXT( mRenderMode, verticesOffset, verticesCount, instanceCount, baseInstance );
#else
		for ( uint32_t j = 0; j < (uint32_t)instanceCount; j++ ) {
			glUniform1i( mIntInstanceID, baseInstance + j );
			glDrawArrays( mRenderMode, verticesOffset, verticesCount );
		}
#endif
	}
/*
	uint32_t iIndices = 0;
	for ( size_t i = 0; i < mObjects.size(); i++ ) {
		const std::vector< std::pair< Image*, uint32_t > >* textures = dynamic_cast<OpenGLES20Object*>(mObjects[i])->textures( mInstance );
		if ( textures and textures->size() > 0 ) {
			glBindTexture( GL_TEXTURE_2D, textures->at(0).second );
		}
		glUniformMatrix4fv( mMatrixObjectID, 1, GL_FALSE, mObjects[i]->matrix()->data() );
#ifdef DRAW_INSTANCED_AVAILABLE
		glDrawElementsInstancedEXT( mRenderMode, mObjects[i]->indicesCount(), GL_UNSIGNED_INT, (void*)(uint64_t)( iIndices * sizeof(uint32_t) ), mObjects[i]->instancesCount() );
#else
		for ( uint32_t j = 0; j < (uint32_t)mObjects[i]->instancesCount(); j++ ) {
			glUniform1i( mIntInstanceID, j );
			glDrawElements( mRenderMode, mObjects[i]->indicesCount(), GL_UNSIGNED_INT, (void*)(uint64_t)( iIndices * sizeof(uint32_t) ) );
		}
#endif
		iIndices += mObjects[i]->indicesCount();
	}
*/
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glUseProgram( 0 );
}


void OpenGLES20Renderer::Look( Camera* cam )
{
	memcpy( mMatrixView->data(), cam->data(), sizeof( float ) * 16 );
	// TODO / TBD : upload matrix to shader here
}


uintptr_t OpenGLES20Renderer::attributeID( const std::string& name )
{
	if ( !mReady ) {
		createPipeline();
	}
	return glGetAttribLocation( mShader, name.c_str() );
}


uintptr_t OpenGLES20Renderer::uniformID( const std::string& name )
{
	if ( !mReady ) {
		createPipeline();
	}
	return glGetUniformLocation( mShader, name.c_str() );
}


void OpenGLES20Renderer::uniformUpload( const uintptr_t id, const float f )
{
	glUseProgram( mShader );
	glUniform1f( id, f );
}


void OpenGLES20Renderer::uniformUpload( const uintptr_t id, const Vector2f& v )
{
	glUseProgram( mShader );
	glUniform2f( id, v.x, v.y );
}


void OpenGLES20Renderer::uniformUpload( const uintptr_t id, const Vector3f& v )
{
	glUseProgram( mShader );
	glUniform3f( id, v.x, v.y, v.z );
}


void OpenGLES20Renderer::uniformUpload( const uintptr_t id, const Vector4f& v )
{
	glUseProgram( mShader );
	glUniform4f( id, v.x, v.y, v.z, v.w );
}


void OpenGLES20Renderer::uniformUpload( const uintptr_t id, const Matrix& v )
{
	glUseProgram( mShader );
	glUniformMatrix4fv( id, 1, GL_FALSE, v.constData() );
}


uint8_t* OpenGLES20Renderer::loadShader( const std::string& filename, size_t* sz )
{
	File* file = new File( filename, File::READ );

	size_t size = file->Seek( 0, File::END );
	file->Rewind();

	uint8_t* data = (uint8_t*)mInstance->Malloc( size + 1 );

	file->Read( data, size );
	data[size] = 0;
	delete file;

	if(sz){
		*sz = size;
	}
	return data;
}
