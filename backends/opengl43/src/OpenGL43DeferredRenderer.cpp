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
#include <math.h>

#include "Window.h"
#include "Instance.h"
#include "OpenGL43Object.h"
#include "OpenGL43Instance.h"
#include "OpenGL43DeferredRenderer.h"
#include "OpenGL43Window.h"
#include "MeshBuilder.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"

extern "C" GE::DeferredRenderer* CreateDeferredRenderer( GE::Instance* instance, uint32_t width, uint32_t height ) {
	return new OpenGL43DeferredRenderer( instance, width, height );
}

OpenGL43DeferredRenderer::OpenGL43DeferredRenderer( Instance* instance, uint32_t width, uint32_t height )
	: OpenGL43Renderer2D( instance, width, height )
	, mCommandListReady( false )
	, mAmbientColor( Vector4f( 0.2f, 0.2f, 0.2f, 1.0f ) )
	, mLightsData( nullptr )
{
	LoadVertexShader( "shaders/deferred_final.vert" );
	LoadFragmentShader( "shaders/deferred_final.frag" );

	MeshBuilder builder( MeshBuilder::Sphere );
	builder.GenerateIndexedVertexArray( mInstance, &mSphereVertices, &mSphereVerticesCount, &mSphereIndices, &mSphereIndicesCount, true );

	mMatrixProjection->Identity();
	mMatrixProjection->Perspective( 60.0f, 16.0f / 9.0f, 0.01f, 1000.0f );
	mMatrixView->Identity();

	Compute();
}


OpenGL43DeferredRenderer::~OpenGL43DeferredRenderer()
{
}


Matrix* OpenGL43DeferredRenderer::projectionMatrix()
{
	return mMatrixProjection;
}


void OpenGL43DeferredRenderer::setAmbientColor( const Vector4f& color )
{
	mAmbientColor = color;

	if ( !mCommandListReady ) {
		return;
	}

	LightData ambiant_data = {
		.position = Vector4f( 0.0f, 0.0f, 0.0f, -1.0f ),
		.direction = Vector4f(),
		.color = mAmbientColor,
		.data = Vector4f( 0.0f, -2.0f ),
	};

	mRenderMutex.lock();
	glBindBuffer( GL_ARRAY_BUFFER, mLightsDataID[ mLightsDataIDi ] );

	LightData* ptr = (LightData*)glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
	memcpy( ptr, &ambiant_data, sizeof(LightData) );

	glUnmapBuffer( GL_ARRAY_BUFFER );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	mRenderMutex.unlock();
}


void OpenGL43DeferredRenderer::AddLight( Light* light )
{
	mCommandListReady = false;
	mLights.emplace_back( light );
}


void OpenGL43DeferredRenderer::AddSunLight( Light* sun_light )
{
	mSunLights.emplace_back( sun_light );
}


void OpenGL43DeferredRenderer::Compute()
{
	if ( !m2DReady ) {
		OpenGL43Renderer2D::Compute();
	}

	glGenFramebuffers( 1, (GLuint*)&mFBO );
	glBindFramebuffer( GL_FRAMEBUFFER, mFBO );

	glGenRenderbuffers( 1, &mColorBuffer );
	glBindRenderbuffer( GL_RENDERBUFFER, mColorBuffer );
// 	glRenderbufferStorageMultisample( GL_RENDERBUFFER, 4, GL_RGBA8, mWidth, mHeight ); //FIXME
	glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, mWidth, mHeight );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER, mColorBuffer );

	glGenRenderbuffers( 1, &mDepthBuffer );
	glBindRenderbuffer (GL_RENDERBUFFER, mDepthBuffer );
// 	glRenderbufferStorageMultisample( GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, mWidth, mHeight );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWidth, mHeight );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER, mDepthBuffer );

	glGenTextures( 1, &mTextureDiffuse );
	glBindTexture( GL_TEXTURE_2D, mTextureDiffuse );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mTextureDiffuse, 0 );

	glGenTextures( 1, &mTextureDepth );
	glBindTexture( GL_TEXTURE_2D, mTextureDepth );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_R16UI, mWidth, mHeight, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, mTextureDepth, 0 );

	glGenTextures( 1, &mTextureNormals );
	glBindTexture( GL_TEXTURE_2D, mTextureNormals );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, mTextureNormals, 0 );

	glGenTextures( 1, &mTexturePositions );
	glBindTexture( GL_TEXTURE_2D, mTexturePositions );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, mTexturePositions, 0 );

	glBindFramebuffer( GL_FRAMEBUFFER, mFBO );
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
	glDrawBuffers( 4, buffers );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
/*
	glGenFramebuffers( 1, (GLuint*)&mFBOColor );
	glBindFramebuffer( GL_FRAMEBUFFER, mFBOColor );
	glBindTexture( GL_TEXTURE_2D, mTextureDiffuse );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mTextureDiffuse, 0 );
	GLenum buffers2[] = { GL_COLOR_ATTACHMENT0_EXT };
	glDrawBuffers( 1, buffers2 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
*/
}


float LightRadius( const Light* light )
{
	float a = 1.0f;
	float b = 0.0f;
	float c = 0.05f;

	float delta = -4.0f * a * c;
	float x = std::sqrt( std::abs( delta ) ) / ( 2.0f * a );
	x += 1.0f / c;

	return x * 0.65f;
}


void OpenGL43DeferredRenderer::ComputeCommandList()
{
	std::vector< Vertex > vertices;
	std::vector< uint32_t > indices;
	uint32_t nFlatLights = 0;
	uint32_t nSphereLights = 0;
	uint32_t nConeLights = 0;
	uint32_t lightsDataCount = 0;
	mLightsDataIndices.clear();

	if ( mLightsData ) {
		mInstance->Free( mLightsData );
	}
	mLightsData = (LightData*)mInstance->Malloc( sizeof(LightData) * ( 1 + mLights.size() ) );

	if ( mMatrixObjects ) {
		mInstance->Free( mMatrixObjects );
	}
	mMatrixObjects = (float*)mInstance->Malloc( sizeof(float) * 16 * mLights.size() );

	for ( size_t i = 0; i < mLights.size(); i++ ) {
		if ( mLights[i]->type() == Light::Point ) {
			if ( mLights[i]->attenuation() == 0.0f ) {
				nFlatLights++;
			} else {
				nSphereLights++;
			}
		}
		if ( mLights[i]->type() == Light::Spot ) {
			nConeLights++;
		}
	}

	// Ambiant light
	LightData data = {
		.position = Vector4f( 0.0f, 0.0f, 0.0f, -1.0f ),
		.direction = Vector4f(),
		.color = mAmbientColor,
		.data = Vector4f( 0.0f, -2.0f ),
	};
	memcpy( &mLightsData[lightsDataCount], &data, sizeof(data) );
	lightsDataCount++;
// 	nFlatLights++;

	for ( size_t i = 0; i < mLights.size(); i++ ) {
		if ( mLights[i]->type() == Light::Point && mLights[i]->attenuation() == 0.0f ) {
			LightData data = {
				.position = Vector4f( mLights[i]->position(), 1.0f ),
				.direction = Vector4f(),
				.color = mLights[i]->color(),
				.data = Vector4f( LightRadius( mLights[i] ), -1.0f, 360.0f, 360.0f )
			};
			mLightsDataIndices.insert( std::make_pair( mLights[i], lightsDataCount ) );
			memcpy( &mLightsData[lightsDataCount], &data, sizeof(data) );
			mLights[i]->setPositionPointer( (Vector3f*)&mLightsData[lightsDataCount].position );
			mLights[i]->setColorPointer( (Vector4f*)&mLightsData[lightsDataCount].color );
			lightsDataCount++;
		}
	}
	for ( size_t i = 0; i < mLights.size(); i++ ) {
		if ( mLights[i]->type() == Light::Point && mLights[i]->attenuation() != 0.0f ) {
			LightData data = {
				.position = Vector4f( mLights[i]->position(), 1.0f ),
				.direction = Vector4f(),
				.color = mLights[i]->color(),
				.data = Vector4f( LightRadius( mLights[i] ), mLights[i]->attenuation(), 360.0f, 360.0f )
			};
			mLightsDataIndices.insert( std::make_pair( mLights[i], lightsDataCount ) );
			memcpy( &mLightsData[lightsDataCount], &data, sizeof(data) );
			mLights[i]->setPositionPointer( (Vector3f*)&mLightsData[lightsDataCount].position );
			mLights[i]->setColorPointer( (Vector4f*)&mLightsData[lightsDataCount].color );
			lightsDataCount++;
		}
	}
	for ( size_t i = 0; i < mLights.size(); i++ ) {
		if ( mLights[i]->type() == Light::Spot ) {
			LightData data = {
				.position = Vector4f( mLights[i]->position(), 1.0f ),
				.direction = Vector4f( mLights[i]->direction(), 1.0f ),
				.color = mLights[i]->color(),
				.data = Vector4f( LightRadius( mLights[i] ), mLights[i]->attenuation(), std::cos( mLights[i]->innerAngle() * 3.14159265359f / 180.0f ), std::cos( mLights[i]->outerAngle() * 3.14159265359f / 180.0f ) )
			};
			mLightsDataIndices.insert( std::make_pair( mLights[i], lightsDataCount ) );
			memcpy( &mLightsData[lightsDataCount], &data, sizeof(data) );
			mLights[i]->setPositionPointer( (Vector3f*)&mLightsData[lightsDataCount].position );
			mLights[i]->setColorPointer( (Vector4f*)&mLightsData[lightsDataCount].color );
			lightsDataCount++;
		}
	}

	Vertex quad[4];
	quad[0].u = 42.0f;
	quad[0].v = 1.0f;
	quad[0].x = -1.0f;
	quad[0].y = -1.0f;

	quad[1].u = 42.0f;
	quad[1].v = 0.0f;
	quad[1].x = 1.0f;
	quad[1].y = 1.0f;

	quad[2].u = 42.0f;
	quad[2].v = 1.0f;
	quad[2].x = -1.0f;
	quad[2].y = 1.0f;

	quad[3].u = 42.0f;
	quad[3].v = 0.0f;
	quad[3].x = 1.0f;
	quad[3].y = -1.0f;

	quad[0].color[0] = quad[1].color[0] = quad[2].color[0] = quad[3].color[0] = 1.0f;
	quad[0].color[1] = quad[1].color[1] = quad[2].color[1] = quad[3].color[1] = 1.0f;
	quad[0].color[2] = quad[1].color[2] = quad[2].color[2] = quad[3].color[2] = 1.0f;
	quad[0].color[3] = quad[1].color[3] = quad[2].color[3] = quad[3].color[3] = 1.0f;

	gDebug() << "nFlatLights : " << nFlatLights << "\n";

	DrawElementsIndirectCommand direct_commands[] = {
		{
			.count = 6,
			.instanceCount = nFlatLights,
			.firstIndex = 0,
			.baseVertex = 0,
			.baseInstance = 1,
		},
		{
			.count = mSphereIndicesCount,
			.instanceCount = nSphereLights,
			.firstIndex = 6,
			.baseVertex = 4,
			.baseInstance = 1 + nFlatLights,
		},
		{
// 			.count = mConeVerticesCount,
			.count = mSphereIndicesCount,
			.instanceCount = nConeLights,
// 			.firstIndex = 6 + mSphereIndicesCount,
			.firstIndex = 6,
//			.baseVertex = 6 + mSphereVerticesCount,
			.baseVertex = 4,
			.baseInstance = 1 + nFlatLights + nSphereLights,
		},
		
	};
	vertices.insert( vertices.end(), quad, &quad[4] );
	vertices.insert( vertices.end(), mSphereVertices, &mSphereVertices[mSphereVerticesCount] );
	indices.emplace_back( 0 );
	indices.emplace_back( 1 );
	indices.emplace_back( 2 );
	indices.emplace_back( 0 );
	indices.emplace_back( 3 );
	indices.emplace_back( 1 );
	indices.insert( indices.end(), mSphereIndices, &mSphereIndices[mSphereIndicesCount] );
// 	vertices.insert( vertices.end(), mConeVertices, mConeVertices + mConeVerticesCount );


	glGenBuffers( 1, &mCommandBuffer );
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mCommandBuffer );
	glBufferData( GL_DRAW_INDIRECT_BUFFER, sizeof(direct_commands), direct_commands, GL_STATIC_DRAW );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(direct_commands) );

	glGenBuffers( 1, &mVBO );
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(Vertex) * vertices.size() );

	glGenBuffers( 1, &mIBO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW );
	((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(uint32_t) * indices.size() );

	mLightsDataIDi = 0;
	glGenVertexArrays( 2, mVAOs );
	glGenBuffers( 2, mLightsDataID );
	for ( int i = 0; i < 2; i++ ) {
		glBindVertexArray( mVAOs[i] );

		glBindBuffer( GL_ARRAY_BUFFER, mLightsDataID[i] );
		glBufferData( GL_ARRAY_BUFFER, sizeof(LightData) * lightsDataCount, mLightsData, GL_DYNAMIC_DRAW );
		glEnableVertexAttribArray( 11 );
		glEnableVertexAttribArray( 12 );
		glEnableVertexAttribArray( 13 );
		glEnableVertexAttribArray( 14 );
		glVertexAttribPointer( 11, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 16, (void*)( sizeof( float ) * 0 ) );
		glVertexAttribPointer( 12, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 16, (void*)( sizeof( float ) * 4 ) );
		glVertexAttribPointer( 13, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 16, (void*)( sizeof( float ) * 8 ) );
		glVertexAttribPointer( 14, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 16, (void*)( sizeof( float ) * 12 ) );
		glVertexAttribDivisor( 11, 1 );
		glVertexAttribDivisor( 12, 1 );
		glVertexAttribDivisor( 13, 1 );
		glVertexAttribDivisor( 14, 1 );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		((OpenGL43Instance*)Instance::baseInstance())->AffectVRAM( sizeof(LightData) * lightsDataCount );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIBO );
		glBindBuffer( GL_ARRAY_BUFFER, mVBO );

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( 0 ) );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 ) );
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 + sizeof( float ) * 4 ) );
		glEnableVertexAttribArray( 3 );
		glVertexAttribPointer( 3, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( float ) * 4 + sizeof( float ) * 4 + sizeof( float ) * 4 ) );
	}

	glBindVertexArray( 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );
	mCommandListReady = true;
}


void OpenGL43DeferredRenderer::Bind()
{
	if ( mAssociatedWindow != nullptr && ( mAssociatedWindow->width() != mWidth || mAssociatedWindow->height() != mHeight ) ) {
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		mWidth = mAssociatedWindow->width();
		mHeight = mAssociatedWindow->height();

		glDeleteFramebuffers( 1, (GLuint*)&mFBO );
		glDeleteRenderbuffers( 1, &mDepthBuffer );
		glDeleteTextures( 1, &mTextureDiffuse );
		glDeleteTextures( 1, &mTextureDepth );
		glDeleteTextures( 1, &mTextureNormals );
		glDeleteTextures( 1, &mTexturePositions );
		Compute();

		Vertex vertices[4];
		memset( vertices, 0, sizeof(vertices) );
		vertices[0].u = 42.0f;
		vertices[0].v = 1.0f;
		vertices[0].x = -1.0f;
		vertices[0].y = -1.0f;

		vertices[1].u = 42.0f;
		vertices[1].v = 0.0f;
		vertices[1].x = 1.0f;
		vertices[1].y = 1.0f;

		vertices[2].u = 42.0f;
		vertices[2].v = 1.0f;
		vertices[2].x = -1.0f;
		vertices[2].y = 1.0f;

		vertices[3].u = 42.0f;
		vertices[3].v = 0.0f;
		vertices[3].x = 1.0f;
		vertices[3].y = -1.0f;

		vertices[0].color[0] = vertices[1].color[0] = vertices[2].color[0] = vertices[3].color[0] = 1.0f;
		vertices[0].color[1] = vertices[1].color[1] = vertices[2].color[1] = vertices[3].color[1] = 1.0f;
		vertices[0].color[2] = vertices[1].color[2] = vertices[2].color[2] = vertices[3].color[2] = 1.0f;
		vertices[0].color[3] = vertices[1].color[3] = vertices[2].color[3] = vertices[3].color[3] = 1.0f;

		glBindBuffer( GL_ARRAY_BUFFER, mVBO );
		glBufferSubData( GL_ARRAY_BUFFER, 0, 4 * sizeof(Vertex), vertices );
	}

	glBindFramebuffer( GL_FRAMEBUFFER, mFBO );
	glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


void OpenGL43DeferredRenderer::Unbind()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


void OpenGL43DeferredRenderer::Render()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	if ( !mCommandListReady ) {
		ComputeCommandList();
	}
/*
	glBindFramebuffer( GL_READ_FRAMEBUFFER, mFBO );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, mFBOColor );
	glBlitFramebuffer( 0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
*/
	const uint32_t binding_proj = 0;
	const uint32_t binding_view = 1;

	glUseProgram( mShader );

	glActiveTexture( GL_TEXTURE0 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, mTextureDiffuse );
	glUniform1i( glGetUniformLocation( mShader, "ge_Texture0" ), 0 );

	glActiveTexture( GL_TEXTURE1 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, mTextureDepth );
	glUniform1i( glGetUniformLocation( mShader, "ge_Texture1" ), 1 );

	glActiveTexture( GL_TEXTURE2 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, mTextureNormals );
	glUniform1i( glGetUniformLocation( mShader, "ge_Texture2" ), 2 );

	glActiveTexture( GL_TEXTURE3 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, mTexturePositions );
	glUniform1i( glGetUniformLocation( mShader, "ge_Texture3" ), 3 );

	glBindBufferBase( GL_UNIFORM_BUFFER, binding_proj, mMatrixProjectionID );
	glBindBufferBase( GL_UNIFORM_BUFFER, binding_view, mMatrixViewID );

	glBindBuffer( GL_UNIFORM_BUFFER, mMatrixViewID );
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, mMatrixView->data() );

// 	mMatrixProjection->Identity();
// 	mMatrixProjection->Perspective( 60.0f, (float)mWidth / (float)mHeight, 0.01f, 1000.0f );
//// 	mMatrixProjection->Orthogonal( 0.0, mWidth, mHeight, 0.0, -2049.0, 2049.0 );
	glBindBuffer( GL_UNIFORM_BUFFER, mMatrixProjectionID );
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, mMatrixProjection->data() );

	glEnable( GL_BLEND );

	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mCommandBuffer );
	mRenderMutex.lock();
	glBindVertexArray( mVAOs[mLightsDataIDi] );
	glDisable( GL_DEPTH_TEST );
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	glMultiDrawElementsIndirect( GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 1, 0 );
	glDepthFunc( GL_LESS );
	mRenderMutex.unlock();

// 	mMatrixProjection->Identity();
// 	mMatrixProjection->Perspective( 60.0f, (float)mWidth / (float)mHeight, 0.01f, 1000.0f );
	glBindBuffer( GL_UNIFORM_BUFFER, mMatrixProjectionID );
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, mMatrixProjection->data() );

// 	glDisable( GL_DEPTH_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mCommandBuffer );
	mRenderMutex.lock();
	glBindVertexArray( mVAOs[mLightsDataIDi] );
// 	glMultiDrawElementsIndirect( GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 2, 0 );
	glMultiDrawElementsIndirect( GL_TRIANGLES, GL_UNSIGNED_INT, (void*)sizeof(DrawElementsIndirectCommand), 2, 0 );
	mRenderMutex.unlock();
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
// 	glEnable( GL_DEPTH_TEST );

	glBindVertexArray( 0 );
	glUseProgram( 0 );
	glActiveTexture( GL_TEXTURE0 );
// 	glBindFramebuffer( GL_FRAMEBUFFER, mFBO );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


void OpenGL43DeferredRenderer::Look( Camera* cam )
{
	memcpy( mMatrixView->data(), cam->data(), sizeof( float ) * 16 );
}


void OpenGL43DeferredRenderer::Update( Light* light )
{
	if ( !mCommandListReady ) {
		return;
	}

	uint32_t offset = 1; // Start just after anbiant
	uint32_t size = mLights.size();
	if ( light ) {
		offset = mLightsDataIndices[ light ];
		size = 1;
		mRenderMutex.lock();
		glBindBuffer( GL_ARRAY_BUFFER, mLightsDataID[ mLightsDataIDi ] );
	} else {
		glBindBuffer( GL_ARRAY_BUFFER, mLightsDataID[ ( ( mLightsDataIDi + 1 ) % 2 ) ] );
	}

// 	glBufferSubData( GL_ARRAY_BUFFER, sizeof(LightData) * offset, sizeof(LightData) * size, &mLightsData[ offset ] );
	LightData* ptr = (LightData*)glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
	memcpy( &ptr[offset], &mLightsData[offset], sizeof(LightData) * size );

	if ( !light ) {
		LightData ambiant_data = {
			.position = Vector4f( 0.0f, 0.0f, 0.0f, -1.0f ),
			.direction = Vector4f(),
			.color = mAmbientColor,
			.data = Vector4f( 0.0f, -2.0f ),
		};
		memcpy( ptr, &ambiant_data, sizeof(LightData) );
	}

	glUnmapBuffer( GL_ARRAY_BUFFER );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	if ( !light ) {
		mRenderMutex.lock();
		mLightsDataIDi = ( mLightsDataIDi + 1 ) % 2;
	}
	mRenderMutex.unlock();
}


uint8_t* OpenGL43DeferredRenderer::loadShader( const std::string& filename, size_t* sz )
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
