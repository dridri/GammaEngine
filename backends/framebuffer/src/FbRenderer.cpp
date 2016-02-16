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
#include "FbObject.h"
#include "FbInstance.h"
#include "FbRenderer.h"
#include "FbWindow.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"

extern "C" GE::Renderer* CreateRenderer( GE::Instance* instance ) {
	return new FbRenderer( instance );
}

bool FbRenderer::s2DActive = false;

FbRenderer::FbRenderer( Instance* instance )
	: mInstance( instance ? instance : Instance::baseInstance() )
	, mMatrixObjects( 0 )
	, mMatrixObjectsSize( 0 )
	, mRenderMode( 0 )
	, mDepthTestEnabled( false )
	, mBlendingEnabled( false )
{

	mMatrixProjection = new Matrix();
	mMatrixProjection->Perspective( 60.0f, 16.0f / 9.0f, 0.01f, 1000.0f );
	mMatrixView = new Matrix();
	mMatrixView->Identity();
}


FbRenderer::~FbRenderer()
{
}


Matrix* FbRenderer::projectionMatrix()
{
	return mMatrixProjection;
}


int FbRenderer::LoadVertexShader( const void* data, size_t size )
{
	return 0;
}


int FbRenderer::LoadFragmentShader( const void* data, size_t size )
{
	return 0;
}


int FbRenderer::LoadVertexShader( const std::string& file )
{
	return 0;
}


int FbRenderer::LoadFragmentShader( const std::string& file )
{
	return 0;
}


void FbRenderer::setRenderMode( int mode )
{
	mRenderMode = mode;
}


void FbRenderer::setDepthTestEnabled( bool en )
{
	mDepthTestEnabled = en;
}


void FbRenderer::setBlendingEnabled(bool en)
{
	mBlendingEnabled = en;
}


void FbRenderer::AddObject( Object* obj )
{
	mObjects.emplace_back( (Object*)obj );
}


void FbRenderer::AddLight( Light* light )
{
	mLights.emplace_back( light );
}


void FbRenderer::Compute()
{
}


void FbRenderer::Draw()
{
}


void FbRenderer::Look( Camera* cam )
{
	memcpy( mMatrixView->data(), cam->data(), sizeof( float ) * 16 );
	// TODO / TBD : upload matrix to shader here
}


uintptr_t FbRenderer::attributeID( const std::string& name )
{
	return 0;
}


uintptr_t FbRenderer::uniformID( const std::string& name )
{
	return 0;
}


void FbRenderer::uniformUpload( const uintptr_t id, const float f )
{
}


void FbRenderer::uniformUpload( const uintptr_t id, const Vector2f& v )
{
}


void FbRenderer::uniformUpload( const uintptr_t id, const Vector3f& v )
{
}


void FbRenderer::uniformUpload( const uintptr_t id, const Vector4f& v )
{
}


void FbRenderer::uniformUpload( const uintptr_t id, const Matrix& v )
{
}
