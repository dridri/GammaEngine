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
#include "FbObject.h"
#include "FbInstance.h"
#include "FbDeferredRenderer.h"
#include "FbWindow.h"
#include "MeshBuilder.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"

extern "C" GE::DeferredRenderer* CreateDeferredRenderer( GE::Instance* instance, uint32_t width, uint32_t height ) {
	return new FbDeferredRenderer( instance, width, height );
}

FbDeferredRenderer::FbDeferredRenderer( Instance* instance, uint32_t width, uint32_t height )
	: FbRenderer2D( instance, width, height )
{
}


FbDeferredRenderer::~FbDeferredRenderer()
{
}


void FbDeferredRenderer::setAmbientColor( const Vector4f& color )
{
}


void FbDeferredRenderer::AddLight( Light* light )
{
}


void FbDeferredRenderer::AddSunLight( Light* sun_light )
{
}


void FbDeferredRenderer::Compute()
{
}


float LightRadius( const Light* light )
{
	return 0.0f;
}


void FbDeferredRenderer::Bind()
{
}


void FbDeferredRenderer::Unbind()
{
}


void FbDeferredRenderer::Render()
{
}


void FbDeferredRenderer::Look( Camera* cam )
{
}


void FbDeferredRenderer::Update( Light* light )
{
}
