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
#include "OpenGL43Object.h"
#include "OpenGL43Instance.h"
#include "OpenGL43Renderer.h"
#include "OpenGL43Window.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"


#ifdef GE_WIN32
#include <windows.h>
static HINSTANCE hOpenGL = 0;
	void* SymLib( const char* name ) {
		if ( !hOpenGL ) {
			hOpenGL = LoadLibrary("opengl32.dll");
		}
		return (void*)GetProcAddress( hOpenGL, name );
	}
#elif defined(GE_LINUX)
typedef void (*__GLXextFuncPtr)(void);
extern "C" __GLXextFuncPtr glXGetProcAddressARB (const GLubyte *);
	void* SymLib( const char* name ) {
		return (void*)glXGetProcAddressARB( (GLubyte*)name );
	}
#else
	void* SymLib( const char* name ) {
		return nullptr;
	}
#endif

// static PFNGLGETTEXTUREHANDLEARBPROC glGetTextureHandle = 0;
// static PFNGLMAKETEXTUREHANDLERESIDENTARBPROC glMakeTextureHandleResident = 0;

extern "C" GE::Renderer* CreateRenderer( GE::Instance* instance ) {
	return new OpenGL43Renderer( instance );
}

static const char* vertex_shader_include = 
	"#version 420 core\n"
	"#extension GL_ARB_shader_draw_parameters : require\n"
	"#extension GL_ARB_bindless_texture : require\n"
	"\n"
	"#define geTexture2D(x) sampler2D( ge_TextureHandlers[ ge_TextureBase + x ].xy )\n"
	"#define geTexture3D(x) sampler3D( ge_TextureHandlers[ ge_TextureBase + x ].xy )\n"
	"\n"
	"layout(location = 0) in vec3 ge_VertexTexcoord;\n"
	"layout(location = 1) in vec4 ge_VertexColor;\n"
	"layout(location = 2) in vec3 ge_VertexNormal;\n"
	"layout(location = 3) in vec3 ge_VertexPosition;\n"
	"layout(location = 7 /* 8 9 10 */) in mat4 ge_ObjectMatrix;\n"
	"layout(location = 11) in int ge_TextureBase;\n"
	"\n"
	"layout (binding=0, std140) uniform ge_Matrices_0\n"
	"{\n"
	"	mat4 ge_ProjectionMatrix;\n"
	"};\n"
	"\n"
	"layout (binding=1, std140) uniform ge_Matrices_1\n"
	"{\n"
	"	mat4 ge_ViewMatrix;\n"
	"};\n"
	"\n"
	"layout (binding=2, std140) uniform ge_Textures_0\n"
	"{\n"
	"	uvec4 ge_TextureHandlers[256];\n"
	"};\n"
;


OpenGL43Renderer::OpenGL43Renderer( Instance* instance )
	: mReady( false )
	, mInstance( instance ? instance : Instance::baseInstance() )
	, mMatrixObjects( 0 )
	, mTotalObjectsInstances( 0 )
	, mRenderMode( GL_TRIANGLES )
	, mDepthTestEnabled( true )
	, mBlendingEnabled( false )
	, mShader( 0 )
	, mVertexShader( 0 )
	, mGeometryShader( 0 )
	, mFragmentShader( 0 )
{
/*
	if ( !glGetTextureHandle ) {
		gDebug() << "Available extensions : " << (char *)glGetString(GL_EXTENSIONS) << "\n";
		glGetTextureHandle = (PFNGLGETTEXTUREHANDLEARBPROC)SymLib( "glGetTextureHandleARB" );
		if ( glGetTextureHandle == nullptr ) {
			glGetTextureHandle = (PFNGLGETTEXTUREHANDLEARBPROC)SymLib( "glGetTextureHandle" );
		}
		if ( glGetTextureHandle == nullptr ) {
			gDebug() << "CRITICAL : glGetTextureHandleARB not found !\n";
// 			exit(0);
		}
	}
	if ( !glMakeTextureHandleResident ) {
		glMakeTextureHandleResident = (PFNGLMAKETEXTUREHANDLERESIDENTARBPROC)SymLib( "glMakeTextureHandleResidentARB" );
		if ( glMakeTextureHandleResident == nullptr ) {
			gDebug() << "CRITICAL : glMakeTextureHandleResidentARB not found !\n";
			exit(0);
		}
	}
*/
	mFloatTimeID = 32;

	mMatrixProjection = new Matrix();
	mMatrixProjection->Perspective( 60.0f, 16.0f / 9.0f, 0.01f, 1000.0f );
	mMatrixView = new Matrix();
	mMatrixView->Identity();
}


OpenGL43Renderer::~OpenGL43Renderer()
{
}


Matrix* OpenGL43Renderer::projectionMatrix()
{
	return mMatrixProjection;
}


int OpenGL43Renderer::LoadVertexShader( const void* data, size_t size )
{
	mReady = false;
	if ( mVertexShader ) {
		glDeleteShader( mVertexShader );
	}

// 	const char* array[2] = { vertex_shader_include, (char*)data };
	mVertexShader = glCreateShader( GL_VERTEX_SHADER );
// 	glShaderSource( mVertexShader, 2, array, NULL );
	glShaderSource( mVertexShader, 1, (const char**)&data, NULL );
	glCompileShader( mVertexShader );
	char log[4096] = "";
	int logsize = 4096;
	glGetShaderInfoLog( mVertexShader, logsize, &logsize, log );
	gDebug() << "vertex compile : " << log << "\n";

	return 0;
}


int OpenGL43Renderer::LoadGeometryShader( const void* data, size_t size )
{
	mReady = false;
	if ( mGeometryShader ) {
		glDeleteShader( mGeometryShader );
	}

// 	const char* array[2] = { vertex_shader_include, (char*)data };
	mGeometryShader = glCreateShader( GL_GEOMETRY_SHADER );
// 	glShaderSource( mGeometryShader, 2, array, NULL );
	glShaderSource( mGeometryShader, 1, (const char**)&data, NULL );
	glCompileShader( mGeometryShader );
	char log[4096] = "";
	int logsize = 4096;
	glGetShaderInfoLog( mGeometryShader, logsize, &logsize, log );
	gDebug() << "Geometry compile : " << log << "\n";

	return 0;
}


int OpenGL43Renderer::LoadFragmentShader( const void* data, size_t size )
{
	mReady = false;
	if ( mFragmentShader ) {
		glDeleteShader( mFragmentShader );
	}

	fDebug( data, size );

	mFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( mFragmentShader, 1, (const char**)&data, NULL );
	glCompileShader( mFragmentShader );
	char log[4096] = "";
	int logsize = 4096;
	glGetShaderInfoLog( mFragmentShader, logsize, &logsize, log );
	gDebug() << "fragment compile : " << log << "\n";

	return 0;
}


int OpenGL43Renderer::LoadVertexShader( const std::string& file )
{
	mReady = false;

	size_t sz = 0;
	uint8_t* data = loadShader( file, &sz );
	int ret = LoadVertexShader( data, sz );
	mInstance->Free( data );

	return ret;
}


int OpenGL43Renderer::LoadGeometryShader( const std::string& file )
{
	mReady = false;

	size_t sz = 0;
	uint8_t* data = loadShader( file, &sz );
	int ret = LoadGeometryShader( data, sz );
	mInstance->Free( data );

	return ret;
}


int OpenGL43Renderer::LoadFragmentShader( const std::string& file )
{
	mReady = false;

	size_t sz = 0;
	uint8_t* data = loadShader( file, &sz );
	int ret = LoadFragmentShader( data, sz );
	mInstance->Free( data );

	return ret;
}


void OpenGL43Renderer::setRenderMode( int mode )
{
	mRenderMode = mode;
}


void OpenGL43Renderer::setDepthTestEnabled( bool en )
{
	mDepthTestEnabled = en;
	if ( en ) {
		glEnable( GL_DEPTH_TEST );
	} else {
		glDisable( GL_DEPTH_TEST );
	}
}


void OpenGL43Renderer::setBlendingEnabled(bool en)
{
	mBlendingEnabled = en;
	if ( en ) {
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	} else {
		glDisable( GL_BLEND );
	}
}


void OpenGL43Renderer::createPipeline()
{
	if ( mShader ) {
		glDeleteProgram( mShader );
	}
	mShader = glCreateProgram();
	glAttachShader( mShader, mVertexShader );
	if ( mGeometryShader ) {
// 		glAttachShader( mShader, mGeometryShader );
	}
	glAttachShader( mShader, mFragmentShader );

	glBindFragDataLocation( mShader, 0, "ge_FragColor" );
	glBindFragDataLocation( mShader, 1, "ge_FragDepth" );
	glBindFragDataLocation( mShader, 2, "ge_FragNormal" );
	glBindFragDataLocation( mShader, 3, "ge_FragPosition" );

	glLinkProgram( mShader );
	glUseProgram( mShader );
	glUseProgram( 0 );

	mReady = true;
}


void OpenGL43Renderer::AddObject( Object* obj )
{
// 	mObjects.emplace_back( (Object*)obj );

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
}


void OpenGL43Renderer::AddLight( Light* light )
{
	mLights.emplace_back( light );
}


void OpenGL43Renderer::Compute()
{
	if ( !mReady ) {
		createPipeline();
	}

	std::vector< DrawElementsIndirectCommand > commands;
	std::vector< Vertex > vertices;
	std::vector< uint32_t > indices;
	std::vector< uint32_t > IDs;
	std::vector< uint32_t > textureBases;
	std::vector< uint64_t > textureHandles;
	uint32_t verticesCount = 0;
	uint32_t indicesCount = 0;
	uint32_t total_instances = 0;

	mTotalObjectsInstances = 0;
	if ( mMatrixObjects ) {
		mInstance->Free( mMatrixObjects );
	}
	for ( size_t i = 0; i < mObjects.size(); i++ ) {
		mTotalObjectsInstances += mObjects[i]->instancesCount();
	}
	mMatrixObjects = (float*)mInstance->Malloc( sizeof(float) * 16 * mTotalObjectsInstances );

	int curr = 0;

	for ( size_t i = 0; i < mObjects.size(); i++ ) {
		DrawElementsIndirectCommand cmd = {
			.count = mObjects[i]->indicesCount(),
			.instanceCount = (uint32_t)mObjects[i]->instancesCount(),
			.firstIndex = indicesCount,
			.baseVertex = verticesCount,
			.baseInstance = (uint32_t)total_instances,
		};
		commands.emplace_back( cmd );

		IDs.emplace_back( (uint32_t)i );
		for ( int j = 0; j < mObjects[i]->instancesCount(); j++ ) {
			mObjects[i]->matrix( j )->setDataPointer( &mMatrixObjects[ 16 * ( total_instances + j )] );
		}

		indices.insert( indices.end(), mObjects[i]->indices(), &mObjects[i]->indices()[mObjects[i]->indicesCount()] );
		vertices.insert( vertices.end(), mObjects[i]->vertices(), &mObjects[i]->vertices()[mObjects[i]->verticesCount()] );
		indicesCount += mObjects[i]->indicesCount();
		verticesCount += mObjects[i]->verticesCount();

		const std::vector< std::pair< Image*, uint32_t > >* textures = ((OpenGL43Object*)mObjects[i])->textures( mInstance );
		if ( textures ) {
			for ( int k = 0; k < mObjects[i]->instancesCount(); k++ ) {
				textureBases.emplace_back( ( textures->size() << 16 ) | ( textureHandles.size() / 2 ) );
			}
			for ( size_t j = 0; j < textures->size(); j++ ) {
				if ( (*textures)[j].first != nullptr ) {
					uint64_t handle = glGetTextureHandleARB( (*textures)[j].second );
					glMakeTextureHandleResidentARB( handle );
					textureHandles.emplace_back( handle );
				} else {
					textureHandles.emplace_back( 0 );
				}
				textureHandles.emplace_back( 0 );
			}
		} else {
			for ( int k = 0; k < mObjects[i]->instancesCount(); k++ ) {
				textureBases.emplace_back( 0x00000000 );
			}
		}
// 		std::cout << "textureBases[" << i << "] = " << std::hex << textureBases.back() << "\n";
		total_instances += cmd.instanceCount;
	}
	gDebug() << "mObjects.size() : " << mObjects.size() << "\n";
	gDebug() << "textureBases.size() : " << textureBases.size() << "\n";
	gDebug() << "textureHandles.size() : " << textureHandles.size() << "\n";
// 	exit(0);

	glGenVertexArrays( 1, &mVAO );
	glBindVertexArray( mVAO );

	glUseProgram( mShader );

	const uint32_t binding_proj = 0;
	const uint32_t binding_view = 1;
	const uint32_t binding_textures = 2;

	glGenBuffers( 1, &mMatrixProjectionID );
	glGenBuffers( 1, &mMatrixViewID );
	glGenBuffers( 1, &mMatrixObjectID );
	glGenBuffers( 1, &mTexturesID );
	glGenBuffers( 1, &mTextureBaseID );

	glBindBuffer( GL_UNIFORM_BUFFER, mMatrixProjectionID );
	glBufferData( GL_UNIFORM_BUFFER, sizeof(float) * 16, mMatrixProjection->data(), GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, binding_proj, mMatrixProjectionID );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(float) * 16 );

	glBindBuffer( GL_UNIFORM_BUFFER, mMatrixViewID );
	glBufferData( GL_UNIFORM_BUFFER, sizeof(float) * 16, mMatrixView->data(), GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, binding_view, mMatrixViewID );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(float) * 16 );

	glBindBuffer( GL_UNIFORM_BUFFER, mTexturesID );
	glBufferData( GL_UNIFORM_BUFFER, sizeof(uint64_t) * textureHandles.size(), textureHandles.data(), GL_STATIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, binding_textures, mTexturesID );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(uint64_t) * textureHandles.size() );

	glUniformBlockBinding( mShader, binding_proj, binding_proj );
	glUniformBlockBinding( mShader, binding_view, binding_view );
	glUniformBlockBinding( mShader, binding_textures, binding_textures );
	glUseProgram( 0 );

	glGenBuffers( 1, &mIBO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indicesCount, indices.data(), GL_STATIC_DRAW );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(uint32_t) * indicesCount );

	glGenBuffers( 1, &mMatrixObjectID );
	glBindBuffer( GL_ARRAY_BUFFER, mMatrixObjectID );
	glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 16 * mTotalObjectsInstances, mMatrixObjects, GL_DYNAMIC_DRAW );
	glEnableVertexAttribArray( 7 );
	glEnableVertexAttribArray( 8 );
	glEnableVertexAttribArray( 9 );
	glEnableVertexAttribArray( 10 );
	glVertexAttribPointer( 7, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 16, (void*)( sizeof( float ) * 0 ) );
	glVertexAttribPointer( 8, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 16, (void*)( sizeof( float ) * 4 ) );
	glVertexAttribPointer( 9, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 16, (void*)( sizeof( float ) * 8 ) );
	glVertexAttribPointer( 10, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 16, (void*)( sizeof( float ) * 12 ) );
	glVertexAttribDivisor( 7, 1 );
	glVertexAttribDivisor( 8, 1 );
	glVertexAttribDivisor( 9, 1 );
	glVertexAttribDivisor( 10, 1 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(float) * 16 * mObjects.size() );

	glBindBuffer( GL_ARRAY_BUFFER, mTextureBaseID );
	glBufferData( GL_ARRAY_BUFFER, sizeof(uint32_t) * textureBases.size(), textureBases.data(), GL_STATIC_DRAW );
	glEnableVertexAttribArray( 11 );
	glVertexAttribIPointer( 11, 1, GL_UNSIGNED_INT, sizeof( uint32_t ), (void*)( 0 ) );
	glVertexAttribDivisor( 11, 1 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(uint32_t) * textureBases.size() );

	glGenBuffers( 1, &mCommandBuffer );
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mCommandBuffer );
	glBufferData( GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawElementsIndirectCommand), commands.data(), GL_STATIC_DRAW );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(DrawElementsIndirectCommand) * commands.size() );

	glGenBuffers( 1, &mVBO );
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferData( GL_ARRAY_BUFFER, verticesCount * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(Vertex) * verticesCount );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( 0 ) );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 ) );
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 + sizeof( float ) * 4 ) );
	glEnableVertexAttribArray( 3 );
	glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 + sizeof( float ) * 4 + sizeof( float ) * 4 ) );

	glBindVertexArray( 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );
}


void OpenGL43Renderer::Draw()
{
	const uint32_t binding_proj = 0;
	const uint32_t binding_view = 1;
	const uint32_t binding_textures = 2;

	if ( mObjects.size() == 0 ) {
		return;
	}
/*
	if ( !mReady ) {
		Compute();
	}
*/

	if ( mDepthTestEnabled ) {
		glEnable( GL_DEPTH_TEST );
	} else {
		glDisable( GL_DEPTH_TEST );
	}
	if ( mBlendingEnabled ) {
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	} else {
		glDisable( GL_BLEND );
	}

	glUseProgram( mShader );
	glBindVertexArray( mVAO );
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mCommandBuffer );

	glBindBufferBase( GL_UNIFORM_BUFFER, binding_proj, mMatrixProjectionID );
	glBindBufferBase( GL_UNIFORM_BUFFER, binding_view, mMatrixViewID );
	glBindBufferBase( GL_UNIFORM_BUFFER, binding_textures, mTexturesID );

	glBindBuffer( GL_UNIFORM_BUFFER, mMatrixProjectionID );
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, mMatrixProjection->data() );

	glBindBuffer( GL_UNIFORM_BUFFER, mMatrixViewID );
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, mMatrixView->data() );

	glBindBuffer( GL_UNIFORM_BUFFER, mMatrixObjectID );
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof(float) * 16 * mTotalObjectsInstances, mMatrixObjects );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );

	glUniform1f( mFloatTimeID, Time::GetSeconds() );

	glMultiDrawElementsIndirect( mRenderMode, GL_UNSIGNED_INT, nullptr, mObjects.size(), 0 );


	glBindVertexArray( 0 );
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );
	glUseProgram( 0 );

}


void OpenGL43Renderer::Look( Camera* cam )
{
	memcpy( mMatrixView->data(), cam->data(), sizeof( float ) * 16 );
	// TODO / TBD : upload matrix to shader here
}


uintptr_t OpenGL43Renderer::attributeID( const std::string& name )
{
	if ( !mReady ) {
		createPipeline();
	}
	return glGetAttribLocation( mShader, name.c_str() );
}


uintptr_t OpenGL43Renderer::uniformID( const std::string& name )
{
	if ( !mReady ) {
		createPipeline();
	}
	return glGetUniformLocation( mShader, name.c_str() );
}


void OpenGL43Renderer::uniformUpload( const uintptr_t id, const float f )
{
	glUseProgram( mShader );
	glUniform1f( id, f );
}


void OpenGL43Renderer::uniformUpload( const uintptr_t id, const Vector2f& v )
{
	glUseProgram( mShader );
	glUniform2f( id, v.x, v.y );
}


void OpenGL43Renderer::uniformUpload( const uintptr_t id, const Vector3f& v )
{
	glUseProgram( mShader );
	glUniform3f( id, v.x, v.y, v.z );
}


void OpenGL43Renderer::uniformUpload( const uintptr_t id, const Vector4f& v )
{
	glUseProgram( mShader );
	glUniform4f( id, v.x, v.y, v.z, v.w );
}


void OpenGL43Renderer::uniformUpload( const uintptr_t id, const Matrix& v )
{
	glUseProgram( mShader );
	glUniformMatrix4fv( id, 1, GL_FALSE, v.constData() );
}


uint8_t* OpenGL43Renderer::loadShader( const std::string& filename, size_t* sz )
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
