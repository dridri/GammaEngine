/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2015  Adrien Aubry <email>
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

#include "PhysicalGraph.h"
#include "Instance.h"
#include "Debug.h"

using namespace GE;

PhysicalGraph::PhysicalGraph( Instance* instance, const Vector3f& gravity )
	: Time()
	, mInstance( instance ? instance : Instance::baseInstance() )
	, mCollider( new PhysicalCollider( this ) )
{
	mBroadphase = new btDbvtBroadphase();
	mCollisionConfiguration = new btDefaultCollisionConfiguration();
	mDispatcher = new btCollisionDispatcher( mCollisionConfiguration );
	mSolver = new btSequentialImpulseConstraintSolver;
	mDynamicsWorld = new btDiscreteDynamicsWorld( mDispatcher, mBroadphase, mSolver, mCollisionConfiguration );
	setGravity( gravity );
}


PhysicalGraph::~PhysicalGraph()
{
	delete mDynamicsWorld;
	delete mSolver;
	delete mDispatcher;
	delete mCollisionConfiguration;
	delete mBroadphase;
}


Instance* PhysicalGraph::instance()
{
	return mInstance;
}


PhysicalCollider* PhysicalGraph::collider()
{
	return mCollider;
}


void PhysicalGraph::setGravity( const Vector3f& g )
{
	mDynamicsWorld->setGravity( btVector3( g.x, g.y, g.z ) );
}


void PhysicalGraph::AddBody( PhysicalBody* body )
{
	body->setTimeParent( this );
	mDynamicsWorld->addRigidBody( body->rigidBody() );
	mBodies.emplace_back( body );
}


void PhysicalGraph::Update( float ndt )
{
	double dt = SlowSync( ndt );
	if ( std::abs( dt ) == 0.0 ) {
		return;
	}
	gDebug() << "step with " << dt;
	mDynamicsWorld->stepSimulation( dt, 1, dt );
}
