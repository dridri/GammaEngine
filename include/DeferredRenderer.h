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

#ifndef DEFERREDRENDERER_H
#define DEFERREDRENDERER_H

#include <vector>
#include "Vector.h"

namespace GE {

class Instance;
class Object;
class Light;
class Camera;
class Matrix;

class DeferredRenderer
{
public:
	virtual ~DeferredRenderer(){};

	virtual void AddLight( Light* light ) = 0;
	virtual void AddSunLight( Light* sun_light ) = 0;
	virtual void setAmbientColor( const Vector4f& color ) = 0;

	virtual void Compute() = 0;
	virtual void Bind() = 0;
	virtual void Unbind() = 0;
	virtual void Render() = 0;
	virtual void Look( Camera* cam ) = 0;

	virtual void Update( Light* light = nullptr ) = 0;

	virtual void AssociateSize( Window* window ) = 0;
};

} // namespace GE

#endif // DEFERREDRENDERER_H