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

#include <cmath>
#include <limits>
#include <stdio.h>
#include "Instance.h"
#include "PhysicalBody.h"
#include "Object.h"
#include "Debug.h"

using namespace GE;

PhysicalBody::PhysicalBody( const Vector3f& pos, float m )
	: Time()
	, mCollideType( NONE )
	, mPosition( pos.x, pos.y, pos.z )
	, mMass( m )
	, mMatrix( Matrix() )
	, mTargetObject( nullptr )
	, mShape( nullptr )
	, mRigidBody( nullptr )
	, mMesh( nullptr )
{
	mMatrix.Translate( pos );
}


PhysicalBody::~PhysicalBody()
{
}


void PhysicalBody::setMass( float m )
{
	mMass = m;
}


void PhysicalBody::ResetBody()
{
	if ( mRigidBody ) {
		delete mRigidBody;
	}

	btVector3 inertia( 0, 0, 0 );
	mShape->calculateLocalInertia( mMass, inertia );

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI( mMass, this, mShape, inertia );
	mRigidBody = new btRigidBody( rigidBodyCI );
}


void PhysicalBody::setPlane( const Vector4f& plane )
{
	if ( mShape ) {
		delete mShape;
	}

	mShape = new btStaticPlaneShape( btVector3( plane.x, plane.y, plane.z ), plane.w );
	ResetBody();
	mCollideType = PLANE;
}


void PhysicalBody::setBox( const Vector3f& min, const Vector3f& max )
{
	if ( mShape ) {
		delete mShape;
	}

	mShape = new btBoxShape( btVector3( ( max.x - min.x ) / 2.0f, ( max.y - min.y ) / 2.0f, ( max.z - min.z ) / 2.0f ) );
	ResetBody();
	mCollideType = BOX;
}


void PhysicalBody::setSphereRadius( float r )
{
	if ( mShape ) {
		delete mShape;
	}

	mShape = new btSphereShape( r );
	ResetBody();
	mCollideType = SPHERE;
}

void PhysicalBody::setMesh( Object* object, bool set_target, bool _static )
{
	if ( mShape ) {
		delete mShape;
	}
	if ( mMesh ) {
		delete mMesh;
	}

	mMesh = new btTriangleIndexVertexArray( object->indicesCount() / 3, (int*)object->indices(), 3 * sizeof( uint32_t ), object->verticesCount(), &object->vertices()[0].x, sizeof( Vertex ) );
	if ( _static ) {
		mShape = new btBvhTriangleMeshShape( mMesh, true, true );
	} else {
		mShape = new btConvexTriangleMeshShape( mMesh );
	}
	ResetBody();
	if ( set_target ) {
		setTarget( object );
	}
	mCollideType = MESH;
}


void PhysicalBody::setTarget( Object* object )
{
	mTargetObject = object;
	memcpy( mTargetObject->matrix()->m, mMatrix.m, sizeof(float) * 16 );
}


void PhysicalBody::setFriction( float f )
{
	if ( mRigidBody ) {
		mRigidBody->setFriction( f );
	}
}


Vector3f PhysicalBody::velocity()
{
	if ( mRigidBody ) {
		return Vector3f( mRigidBody->getLinearVelocity().x(), mRigidBody->getLinearVelocity().y(), mRigidBody->getLinearVelocity().z() );
	}
	return Vector3f();
}


Vector3f PhysicalBody::position()
{
	return Vector3f( mMatrix.m[3], mMatrix.m[7], mMatrix.m[11] );
}


Matrix& PhysicalBody::matrix()
{
// 	btTransform trans;
// 	mRigidBody->getMotionState()->getWorldTransform( trans );
// 	trans.getOpenGLMatrix( mMatrix.data() );
	return mMatrix;
}


Matrix PhysicalBody::rotationMatrix()
{
	Matrix rot = mMatrix;
	rot.m[3] = rot.m[7] = rot.m[11] = 0.0f;
	return rot;
}


Matrix PhysicalBody::inverseRotationMatrix()
{
	Matrix rot = mMatrix;
	rot.m[3] = rot.m[7] = rot.m[11] = 0.0f;
	// TODO : inverse matrix
	return rot;
}


PhysicalBody::CollideType PhysicalBody::collisionType()
{
	return mCollideType;
}


Vector3f* PhysicalBody::collisionMesh( uint32_t* nVerts )
{
	return nullptr;
}


btRigidBody* PhysicalBody::rigidBody()
{
	return mRigidBody;
}


void PhysicalBody::ResetForces()
{
	if ( mRigidBody ) {
		mRigidBody->clearForces();
	}
}


void PhysicalBody::ResetTorque()
{
	if ( mRigidBody ) {
		mRigidBody->setAngularVelocity( btVector3( 0.0f, 0.0f, 0.0f ) );
	}
}


void PhysicalBody::ApplyForce( const Vector3f& f )
{
	if ( mRigidBody ) {
		mRigidBody->applyCentralForce( btVector3( f.x, f.y, f.z ) );
	}
}


void PhysicalBody::ApplyTorque( const Vector3f& t )
{
	if ( mRigidBody ) {
		mRigidBody->applyTorque( btVector3( t.x, t.y, t.z ) );
	}
}


void PhysicalBody::ApplyGravity( PhysicalBody* other, bool apply_to_other )
{
	const double G = 6.67234e-11;
	Vector3f dir = other->mPosition - mPosition;
	Vector3f force;
	
	double d = std::sqrt( dir.x * dir.x + dir.y * dir.y + dir.z * dir.z );
	if ( d <= 0.5 || d == std::numeric_limits<double>::infinity() ) {
		return;
	}
	double F = G * ( ( mMass * other->mMass ) / ( d * d ) );

	force = dir;
	force.normalize();
	force *= F;

	ApplyForce( force );
	if ( apply_to_other ) {
		other->ApplyForce( -force );
	}
}


void PhysicalBody::getWorldTransform( btTransform& worldTrans ) const
{
	btTransform t( btQuaternion( 0, 0, 0, 1 ), btVector3( mPosition.x, mPosition.y, mPosition.z ) );
	worldTrans = t;
}


void PhysicalBody::setWorldTransform( const btTransform& worldTrans )
{
	if ( mTargetObject ) {
		worldTrans.getOpenGLMatrix( mTargetObject->matrix()->m );
		mTargetObject->matrix()->m[15] = 1.0f;
		memcpy( mMatrix.m, mTargetObject->matrix()->m, sizeof(float) * 16 );
	} else {
		worldTrans.getOpenGLMatrix( mMatrix.m );
		mMatrix.m[15] = 1.0f;
	}
}
