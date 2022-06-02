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

#ifndef MATRIX_H
#define MATRIX_H

#include "Vector.h"

namespace GE {

class Matrix
{
public:
	Matrix();
	virtual ~Matrix();

	void Identity();
	void Perspective( float fov, float aspect, float zNear, float zFar );
	void Orthogonal( float left, float right, float bottom, float top, float zNear, float zFar );
	void LookAt( const Vector3f& eye, const Vector3f& center, const Vector3f& up, bool centered = true );
	
	void Translate( const Vector3f& v );
	void Translate( float x, float y, float z );
	void Scale( float x, float y, float z );
	void RotateX( float a );
	void RotateY( float a );
	void RotateZ( float a );

	void setDataPointer( float* d, bool keep_current_data = true );
	float* data();
	float* constData() const;

// 	Matrix operator*( Matrix& other );
	void operator*=( const Matrix& other );
	void operator=( const Matrix& other );

// protected:
public:
	float* m;

protected:
	bool mAllocedData;
};

Matrix operator*( const Matrix& m1, const Matrix& m2 );
Vector4f operator*( const Matrix& m, const Vector4f& v );

} // namespace GE

#endif // MATRIX_H
