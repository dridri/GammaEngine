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

#ifndef CAMERA_H
#define CAMERA_H

#include "Time.h"
#include "Matrix.h"

namespace GE {


class Camera : protected Time
{
public:
	typedef enum {
		Free,
		FPS,
	} Mode;

	Camera( const Mode& mode = FPS );
	~Camera();

	void UpVector( const Vector3f& up );
	void LookAt( const Vector3f& pos, const Vector3f& center );
	void Translate( const Vector3f& t );
	void WalkForward( float speed = 1.0f );
	void WalkBackward( float speed = 1.0f );
	void WalkLeft( float speed = 1.0f );
	void WalkRight( float speed = 1.0f );
	void RotateH( const float v, float speed = 1.0f );
	void RotateV( const float v, float speed = 1.0f );
	void setPosition( const Vector3f& pos );
	void setInertia( const float inertia );
	void setRotationInertia( const float inertia );

	const Vector3f& position();
	const Vector3f& lookPoint();
	Vector2f rotation();
	Vector3f direction();
	float* data();

protected:
	Mode mMode;
	Matrix mMatrix;
	Vector3f mPosition;
	Vector3f mLookPoint;
	Vector3f mUpVector;
	float mRotV; // in rad
	float mRotH; // in rad
	Vector3f mInertia;
	Vector2f mTorque;
	float mInertiaDuration;
	float mTorqueDuration;
	float mInertiaFactor;
	float mTorqueFactor;

	void Update();
};

} // namespace GE

#endif // CAMERA_H
