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

#ifndef GE_SCENE_H
#define GE_SCENE_H

#include <vector>
#include <string>

#include "Renderer.h"
#include "Camera.h"

namespace GE {

class Scene
{
public:
	Scene( const std::string& filename = "" );
	~Scene();

	void AddRenderer( Renderer* renderer );
	void RemoveRenderer( Renderer* renderer );
	Renderer* renderer( std::string name );

	void Draw( Camera* camera );

protected:
	std::vector< Renderer* > mRenderers;
};

}

#endif // GE_SCENE_H
