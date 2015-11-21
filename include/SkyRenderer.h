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

#ifndef SKYRENDERER_H
#define SKYRENDERER_H

#include <vector>
#include "Instance.h"
#include "Renderer.h"
#include "MeshBuilder.h"

namespace GE {

class Object;
class Light;
class Camera;
class Matrix;
class Image;

class SkyRenderer
{
public:
	SkyRenderer( Instance* instance, float domeRadius );
	~SkyRenderer();

	void AddLight( Light* light );
	void Render( Camera* cam );

	void AssociateSize( Window* window ) { mAssociatedWindow = window; };

protected:
	bool BuilderRemoveCB( MeshBuilder::Face* face );
	void BuilderModCB( MeshBuilder::Face* face );
	Renderer* mRenderer;
	Window* mAssociatedWindow;
	Object* mDome;
	float mDomeRadius;
	std::vector< Light* > mLights;
	uintptr_t mLightPosID;
	uintptr_t mRandTextureID;
	Image* mRandTexture;
};

} // namespace GE

#endif // SKYRENDERER_H
