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

#ifndef GE_PHYSICALGRAPH_H
#define GE_PHYSICALGRAPH_H

#include <list>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include "PhysicalCollider.h"
#include "PhysicalBody.h"

namespace GE
{

class Instance;

class PhysicalGraph : protected Time
{
public:
	PhysicalGraph( Instance* instance = nullptr, const Vector3f& gravity = Vector3f() );
	~PhysicalGraph();

	void setGravity( const Vector3f& g );

	PhysicalCollider* collider();

	void AddBody( PhysicalBody* body );

	void Update( float ndt = 1.0f / 100.0f );

	Instance* instance();

protected:
	Instance* mInstance;
	PhysicalCollider* mCollider;
	std::list< PhysicalBody* > mBodies;

    btBroadphaseInterface* mBroadphase;
    btDefaultCollisionConfiguration* mCollisionConfiguration;
    btCollisionDispatcher* mDispatcher;
    btSequentialImpulseConstraintSolver* mSolver;
    btDiscreteDynamicsWorld* mDynamicsWorld;
};

} // namespace GE

#endif // GE_PHYSICALGRAPH_H
