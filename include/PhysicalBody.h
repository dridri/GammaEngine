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

#ifndef GE_PHYSICALOBJECT_H
#define GE_PHYSICALOBJECT_H

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include "Vector.h"
#include "Quaternion.h"
#include "Matrix.h"
#include "Time.h"

namespace GE
{

class Object;

class PhysicalBody : public Time, public btMotionState
{
public:
	typedef enum {
		NONE = 0,
		PLANE = 1,
		BOX = 2,
		SPHERE = 2,
		MESH = 10
	} CollideType;

	PhysicalBody( const Vector3f& pos = Vector3f(), float mass = 1.0f );
	~PhysicalBody();

	void setMass( float m );
	void setFriction( float f );

	void setPlane( const Vector4f& plane );
	void setBox( const Vector3f& min, const Vector3f& max );
	void setSphereRadius( float r );
	void setMesh( Object* object, bool set_target = true, bool _static = false );
	void setTarget( Object* object );

	Vector3f velocity();
	Vector3f position();
	Matrix& matrix();
	Matrix rotationMatrix();
	Matrix inverseRotationMatrix();
	CollideType collisionType();
	Vector3f* collisionMesh( uint32_t* nVerts = nullptr );

	btRigidBody* rigidBody();

	void ResetForces();
	void ResetTorque();

	void ApplyForce( const Vector3f& f );
	void ApplyTorque( const Vector3f& t );
	void ApplyGravity( PhysicalBody* other, bool apply_to_other = true );

protected:
	void ResetBody();
	virtual void getWorldTransform( btTransform& worldTrans ) const;
	virtual void setWorldTransform( const btTransform& worldTrans );

	CollideType mCollideType;
	Vector3f mPosition;
	float mMass;
	Matrix mMatrix;
	Object* mTargetObject;

	btCollisionShape* mShape;
	btRigidBody* mRigidBody;
	btStridingMeshInterface* mMesh;
};


}

#endif // GE_PHYSICALOBJECT_H
