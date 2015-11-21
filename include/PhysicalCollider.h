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

#ifndef GE_PHYSICALCOLLIDER_H
#define GE_PHYSICALCOLLIDER_H

#include <vector>
#include <tuple>

#include "PhysicalBody.h"

namespace GE
{

class PhysicalGraph;

class PhysicalCollider
{
public:
	PhysicalCollider( PhysicalGraph* parent );
	~PhysicalCollider();

	void Collide( PhysicalBody* a, PhysicalBody* b );

	Vector3f* minkowskiDifference( PhysicalBody* a, PhysicalBody* b, uint32_t* nVerts );

private:
	std::tuple< Vector3f, uint32_t, uint32_t > Support( const Vector3f* A, uint32_t nVA, const Vector3f* B, uint32_t nVB, const Vector3f& dir );

	PhysicalGraph* mParent;
};

}

#endif // GE_PHYSICALCOLLIDER_H
