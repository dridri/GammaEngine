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

#ifndef GE_LIGHT_H
#define GE_LIGHT_H

#include "Vector.h"

namespace GE
{

class Light
{
public:
	typedef enum Type {
		Point,
		Spot,
	} Type;

	Light( const Vector4f& color, const Vector3f& position, float attenuation = 0.05f );
	Light( const Vector4f& color, const Vector3f& position, const Vector3f& direction, float innerAngle = 45.0f, float outerAngle = 70.0f, float attenuation = 0.02f );
	~Light();

	Type type() const;
	const Vector4f& color() const;
	const Vector3f& position() const;
	const Vector3f& direction() const;
	float attenuation() const;
	float innerAngle() const;
	float outerAngle() const;

	void setPosition( const Vector3f& pos );
	void setColor( const Vector4f& color );

	void setPositionPointer( Vector3f* ptr );
	void setColorPointer( Vector4f* ptr );

private:
	Type mType;
	Vector4f* mColor;
	Vector3f* mPosition;
	Vector3f mDirection;
	float mInnerAngle;
	float mOuterAngle;
	float mAttenuation;
	bool mColorPtr;
	bool mPositionPtr;
};

} // namespace GE

#endif // GE_LIGHT_H
