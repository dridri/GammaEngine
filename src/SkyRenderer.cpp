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

#include <vector>
#include <unordered_map>
#include "Instance.h"
#include "Window.h"
#include "SkyRenderer.h"
#include "Object.h"
#include "Light.h"
#include "Camera.h"
#include "MeshBuilder.h"
#include "Image.h"
#include "Debug.h"

using namespace GE;

bool SkyRenderer::BuilderRemoveCB( MeshBuilder::Face* face )
{
	return face->p0().z >= 0.0f || face->p1().z >= 0.0f || face->p2().z >= 0.0f;
}


void SkyRenderer::BuilderModCB( MeshBuilder::Face* face )
{
	Vector3f p0 = face->p0();
	Vector3f p1 = face->p1();
	Vector3f p2 = face->p2();
	p0.z = std::max( p0.z, 0.0f );
	p1.z = std::max( p1.z, 0.0f );
	p2.z = std::max( p2.z, 0.0f );
	*face = MeshBuilder::Face( p0, p1, p2 );
}


SkyRenderer::SkyRenderer( Instance* instance, float domeRadius )
	: mRenderer( instance->CreateRenderer() )
	, mAssociatedWindow( nullptr )
	, mDome( nullptr )
	, mDomeRadius( domeRadius )
{
	mRenderer->projectionMatrix()->Perspective( 60.0f, 16.0f / 9.0f, 1.0f, domeRadius + 10.0f );

	MeshBuilder builder( MeshBuilder::Sphere, Vector3f( domeRadius, domeRadius, domeRadius ), 4 );
	builder.Translate( { 0.0f, 0.0f, -domeRadius + domeRadius / 18.0f } );
// 	builder.RemoveFaces( (MeshBuilder::MeshBuilderRemoveCb)&SkyRenderer::BuilderRemoveCB ); // TODO : use std::function
	builder.Translate( { 0.0f, 0.0f, domeRadius - domeRadius / 18.0f } );
	builder.Tesselate( MeshBuilder::Normalize );
	builder.Tesselate( MeshBuilder::Normalize );
	builder.Tesselate( MeshBuilder::Normalize );
	builder.Translate( { 0.0f, 0.0f, -domeRadius + domeRadius / 18.0f } );
// 	builder.SinglePassFaces( (MeshBuilder::MeshBuilderPassCb)&SkyRenderer::BuilderModCB ); // TODO : use std::function

	Vertex* verts = nullptr;
	uint32_t nVerts = 0;
	uint32_t* indices = nullptr;
	uint32_t nIndices = 0;
	builder.GenerateIndexedVertexArray( instance, &verts, &nVerts, &indices, &nIndices, true );
	mDome = instance->CreateObject( verts, nVerts, indices, nIndices );
/*
	mRandTexture = new Image( 256, 256 );
	for ( int j = 0; j < 256; j++ ) {
		for ( int i = 0; i < 256; i++ ) {
			mRandTexture->data()[ j * 256 + i ] = ( ( rand() % 256 ) << 24 ) | ( ( rand() % 256 ) << 16 ) | ( ( rand() % 256 ) << 8 ) | ( rand() % 256 );
		}
	}
	mDome->setTexture( instance, 0, mRandTexture );
*/
	mRenderer->LoadVertexShader( "shaders/skydome.vert" );
	mRenderer->LoadFragmentShader( "shaders/skydome.frag" );
	mRenderer->AddObject( mDome );
	mRenderer->Compute();
	mLightPosID = mRenderer->uniformID( "v3SunPos" );
	mRandTextureID = mRenderer->uniformID( "ge_RandTexture" );
}


SkyRenderer::~SkyRenderer()
{
}


void SkyRenderer::AddLight( Light* light )
{
	mLights.emplace_back( light );
}


void SkyRenderer::Render( Camera* camera )
{
	if ( mAssociatedWindow != nullptr ) {
		mRenderer->projectionMatrix()->Perspective( 60.0f, (float)mAssociatedWindow->width() / mAssociatedWindow->height(), 1.0f, mDomeRadius + 10.0f );
	}

	// TODO : Handle multiple mLight
	mRenderer->uniformUpload( mLightPosID, mLights[0]->position() );

	mRenderer->Look( camera );
	mRenderer->Draw();
}

