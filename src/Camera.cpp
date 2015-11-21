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
#include "Camera.h"

namespace GE {

Camera::Camera( const Mode& mode )
	: Time()
	, mMode( mode )
	, mMatrix( Matrix() )
	, mPosition( Vector3f( 0.0f, 0.0f, 0.0f ) )
	, mLookPoint( Vector3f( 1.0f, 0.0f, 0.0f ) )
	, mUpVector( Vector3f( 0.0f, 0.0f, 1.0f ) )
	, mRotV( 0.0f )
	, mRotH( 0.0f )
	, mInertia( Vector3f( 0.0f, 0.0f, 0.0f ) )
	, mTorque( Vector2f( 0.0f, 0.0f ) )
	, mInertiaDuration( 0.0f )
	, mTorqueDuration( 0.0f )
	, mInertiaFactor( 0.0f )
	, mTorqueFactor( 0.0f )
{
}


Camera::~Camera()
{
}


void Camera::UpVector( const Vector3f& up )
{
	mUpVector = up;
}


void Camera::LookAt( const Vector3f& pos, const Vector3f& center )
{
	mPosition = pos;
	mLookPoint = center;
}


void Camera::setInertia( const float inertia )
{
	mInertiaFactor = inertia;
}


void Camera::setRotationInertia( const float inertia )
{
	mTorqueFactor = inertia;
}


void Camera::Translate( const Vector3f& t )
{
	mPosition += t;
	mLookPoint += t;
}


void Camera::WalkForward( float speed )
{
	float dt = Sync();

	float x = std::cos( mRotH ) * speed * dt;
	float y = std::sin( mRotH ) * speed * dt;

	mPosition.x += x;
	mPosition.y += y;

	mLookPoint.x += x;
	mLookPoint.y += y;

	mInertia.x = x * speed;
	mInertia.y = y * speed;
	mTorqueDuration += speed;
}


void Camera::WalkBackward( float speed )
{
	float dt = Sync();

	float x = std::cos( mRotH ) * speed * dt;
	float y = std::sin( mRotH ) * speed * dt;

	mPosition.x -= x;
	mPosition.y -= y;

	mLookPoint.x -= x;
	mLookPoint.y -= y;

	mInertia.x = -x * speed;
	mInertia.y = -y * speed;
}


void Camera::WalkLeft( float speed )
{
	float dt = Sync();

	float x = std::cos( mRotH ) * speed * dt;
	float y = std::sin( mRotH ) * speed * dt;

	mPosition.x -= y;
	mPosition.y += x;

	mLookPoint.x -= y;
	mLookPoint.y += x;

	mInertia.x = -y * speed;
	mInertia.y = x * speed;
}


void Camera::WalkRight( float speed )
{
	float dt = Sync();

	float x = std::cos( mRotH ) * speed * dt;
	float y = std::sin( mRotH ) * speed * dt;

	mPosition.x += y;
	mPosition.y -= x;

	mLookPoint.x += y;
	mLookPoint.y -= x;

	mInertia.x = y * speed;
	mInertia.y = -x * speed;
}


void Camera::RotateH( const float v, float speed )
{
	mRotH += v * speed;

	mTorque.x += v * speed;
}


void Camera::RotateV( const float v, float speed )
{
	mRotV += v * speed;

	mTorque.y += v * speed;
}


void Camera::Update()
{
/*
	mInertia *= mInertiaFactor;
	mPosition.x += mInertia.x;
	mPosition.y += mInertia.y;
	mPosition.z += mInertia.z;

	mTorque *= mTorqueFactor;
	mRotH *= mTorque.x;
	mRotV *= mTorque.y;
*/
	if ( mMode == FPS ) {
		float rtemp = 1000.0f * std::cos( mRotV );
		mLookPoint.z = mPosition.z + 1000.0f * std::sin( mRotV );
		mLookPoint.x = mPosition.x + rtemp * std::cos( mRotH );
		mLookPoint.y = mPosition.y + rtemp * std::sin( mRotH );
	}
}


Vector3f Camera::direction()
{
	Vector3f ret = mLookPoint - mPosition;
	ret.normalize();
	return ret;
}


float* Camera::data()
{
	Update();
	mMatrix.Identity();
	mMatrix.LookAt( mPosition, mLookPoint, mUpVector );
	return mMatrix.data();
}

} // namespace GE
