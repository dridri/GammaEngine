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

#include "Light.h"

using namespace GE;

Light::Light( const Vector4f& color, const Vector3f& position, float attenuation )
	: mType( Point )
	, mColor( new Vector4f( color ) )
	, mPosition( new Vector3f( position ) )
	, mInnerAngle( 360.0f )
	, mOuterAngle( 360.0f )
	, mAttenuation( attenuation )
	, mColorPtr( false )
	, mPositionPtr( false )
{
}


Light::Light( const Vector4f& color, const Vector3f& position, const Vector3f& direction, float innerAngle, float outerAngle, float attenuation )
	: mType( Spot )
	, mColor( new Vector4f( color ) )
	, mPosition( new Vector3f( position ) )
	, mDirection( direction )
	, mInnerAngle( innerAngle )
	, mOuterAngle( outerAngle )
	, mAttenuation( attenuation )
	, mColorPtr( false )
	, mPositionPtr( false )
{
	mDirection.normalize();
}


Light::~Light()
{
}


Light::Type Light::type() const
{
	return mType;
}


const Vector4f& Light::color() const
{
	return *mColor;
}


const Vector3f& Light::position() const
{
	return *mPosition;
}


const Vector3f& Light::direction() const
{
	return mDirection;
}


float Light::attenuation() const
{
	return mAttenuation;
}


float Light::innerAngle() const
{
	return mInnerAngle;
}


float Light::outerAngle() const
{
	return mOuterAngle;
}


void Light::setPosition( const Vector3f& pos )
{
	mPosition->x = pos.x;
	mPosition->y = pos.y;
	mPosition->z = pos.z;
}


void Light::setColor( const Vector4f& color )
{
	mColor->x = color.x;
	mColor->y = color.y;
	mColor->z = color.z;
	mColor->w = color.w;
}


void Light::setPositionPointer( Vector3f* ptr )
{
	if ( !mPositionPtr ) {
		delete mPosition;
	}
	mPositionPtr = true;
	mPosition = ptr;
}


void Light::setColorPointer( Vector4f* ptr )
{
	if ( !mColorPtr ) {
		delete mColor;
	}
	mColorPtr = true;
	mColor = ptr;
}
