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

/* WARNING : Deferred Renderer is not supported by OpenGL ES 2.0 */

#include <fstream>
#include <vector>
#include <math.h>

#include "Window.h"
#include "Instance.h"
#include "OpenGLES20Object.h"
#include "OpenGLES20Instance.h"
#include "OpenGLES20DeferredRenderer.h"
#include "OpenGLES20Window.h"
#include "MeshBuilder.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"

extern "C" GE::DeferredRenderer* CreateDeferredRenderer( GE::Instance* instance, uint32_t width, uint32_t height ) {
	return new OpenGLES20DeferredRenderer( instance, width, height );
}

OpenGLES20DeferredRenderer::OpenGLES20DeferredRenderer( Instance* instance, uint32_t width, uint32_t height )
	: OpenGLES20Renderer2D( instance, width, height )
	, mCommandListReady( false )
	, mAmbientColor( Vector4f( 0.2f, 0.2f, 0.2f, 1.0f ) )
	, mLightsData( nullptr )
{
}


OpenGLES20DeferredRenderer::~OpenGLES20DeferredRenderer()
{
}


void OpenGLES20DeferredRenderer::setAmbientColor( const Vector4f& color )
{
}


void OpenGLES20DeferredRenderer::AddLight( Light* light )
{
}


void OpenGLES20DeferredRenderer::AddSunLight( Light* sun_light )
{
}


void OpenGLES20DeferredRenderer::Compute()
{
}


float LightRadius( const Light* light )
{
	return 0.0f;
}


void OpenGLES20DeferredRenderer::ComputeCommandList()
{
}


void OpenGLES20DeferredRenderer::Bind()
{
}


void OpenGLES20DeferredRenderer::Unbind()
{
}


void OpenGLES20DeferredRenderer::Render()
{
}


void OpenGLES20DeferredRenderer::Look( Camera* cam )
{
}


void OpenGLES20DeferredRenderer::Update( Light* light )
{
}


uint8_t* OpenGLES20DeferredRenderer::loadShader( const std::string& filename, size_t* sz )
{
	if ( sz ) {
		*sz = 0;
	}
	return nullptr;
}
