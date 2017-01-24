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

#ifndef FRAMEBUFFERDEFERREDRENDERER_H
#define FRAMEBUFFERDEFERREDRENDERER_H

#include <mutex>
#include <vector>
#include <unordered_map>
#include "DeferredRenderer.h"
#include "FbRenderer2D.h"
#include "Object.h"
#include "Light.h"

namespace GE {
	class Instance;
	class Object;
	class Camera;
	class Matrix;
}
using namespace GE;

class FbDeferredRenderer : public DeferredRenderer, private FbRenderer2D
{
public:
	FbDeferredRenderer( Instance* instance, uint32_t width, uint32_t height );
	~FbDeferredRenderer();

	virtual void AddLight( Light* light );
	virtual void AddSunLight( Light* sun_light );
	virtual void setAmbientColor( const Vector4f& color );

	virtual void Compute();
	virtual void Bind();
	virtual void Unbind();
	virtual void Render();
	virtual void Look( Camera* cam );

	virtual Matrix* projectionMatrix();
	virtual void Update( Light* light = nullptr );

	virtual void AssociateSize( Window* window ) { mAssociatedWindow = window; };

private:
};

#endif // FRAMEBUFFERDEFERREDRENDERER_H
