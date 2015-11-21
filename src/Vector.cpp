/*
 * <one line to give the library's name and an idea of what it does.>
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

#if ( defined(GE_VECTOR_CPP_INC) || !( defined(GE_ANDROID) || defined(GE_IOS) || defined(GE_RELEASE) ) )

#include "Instance.h"
#include "Vector.h"
#include "Debug.h"
#include <cmath>

#define VEC_OP( r, a, op, b ) \
	r x = a x op b x; \
	if ( n > 1 ) { \
		r y = a y op b y; \
		if ( n > 2 ) { \
			r z = a z op b z; \
			if ( n > 3 ) { \
				r w = a w op b w; \
			} \
		} \
	}

#define VEC_IM( r, a, op, im ) \
	r x = a x op im; \
	if ( n > 1 ) { \
		r y = a y op im; \
		if ( n > 2 ) { \
			r z = a z op im; \
			if ( n > 3 ) { \
				r w = a w op im; \
			} \
		} \
	}
	
#define VEC_ADD( r, a, op, b ) \
	r += a x op b x; \
	if ( n > 1 ) { \
		r += a y op b y; \
		if ( n > 2 ) { \
			r += a z op b z; \
			if ( n > 3 ) { \
				r += a w op b w; \
			} \
		} \
	}

#define VEC_ADD_IM( r, a, op, im ) \
	r += a x op im; \
	if ( n > 1 ) { \
		r += a y op im; \
		if ( n > 2 ) { \
			r += a z op im; \
			if ( n > 3 ) { \
				r += a w op im; \
			} \
		} \
	}

using namespace GE;

//template <typename T, int n> Vector<T,n>::Vector( T x, T y, T z, T w ) : x( x ), y( y ), z( z ), w( w ) {}


template <typename T, int n> Vector<T,n>& Vector<T,n>::operator=( const Vector< T, n >& other )
{
	VEC_IM( this-> , other. , + , 0 );
	return *this;
}

template <typename T, int n> void Vector<T,n>::normalize() {
	T add = 0;
	VEC_ADD( add, this-> , * , this-> );
	T l = std::sqrt( add );
	if ( l > 0.00001f ) {
		T il = 1 / l;
		VEC_IM( this-> , this-> , * , il );
	}
}


template <typename T, int n> T Vector<T,n>::length() {
	T add = 0;
	VEC_ADD( add, this-> , * , this-> );
	return std::sqrt( add );
}


template <typename T, int n> T Vector<T,n>::operator[]( int i ) const
{
	return ( &x )[i];
}


template <typename T, int n> T& Vector<T,n>::operator[]( int i )
{
	return ( &x )[i];
}


template <typename T, int n> Vector<T,n> Vector<T,n>::operator-() const
{
	Vector<T, n> ret;
	VEC_IM( ret. , - this-> , + , 0.0f );
	return ret;
}


template <typename T, int n> void Vector<T,n>::operator+=( const Vector<T,n>& v ) {
	VEC_OP( this-> , this-> , + , v. );
}


template <typename T, int n> void Vector<T,n>::operator-=( const Vector<T,n>& v ) {
	VEC_OP( this-> , this-> , - , v. );
}

template <typename T, int n> void Vector<T,n>::operator*=( T v ) {
	VEC_IM( this-> , this-> , * , v );
}


template <typename T, int n> Vector<T,n> Vector<T,n>::operator+( const Vector<T,n>& v ) const {
	Vector<T, n> ret;
	VEC_OP( ret. , this-> , + , v. );
	return ret;
}

template <typename T, int n> Vector<T,n> Vector<T,n>::operator-( const Vector<T,n>& v ) const {
	Vector<T, n> ret;
	VEC_OP( ret. , this-> , - , v. );
	return ret;
}

template <typename T, int n> Vector<T,n> Vector<T,n>::operator*( T im ) const {
	Vector<T, n> ret;
	VEC_IM( ret. , this-> , * , im );
	return ret;
}

template <typename T, int n> T Vector<T,n>::operator*( const Vector<T,n>& v ) const {
	T ret = 0;
	VEC_ADD( ret, this-> , * , v. );
	return ret;
}

template <typename T, int n> Vector<T,n> Vector<T,n>::operator^( const Vector<T,n>& v ) const {
	Vector<T, n> ret;
	for ( int i = 0; i < n; i++ ) { // TODO : direct op
		T a = ( &x )[ ( i + 1 ) % n ];
		T b = ( &v.x )[ ( i + 2 ) % n ];
		T c = ( &x )[ ( i + 2 ) % n ];
		T d = ( &v.x )[ ( i + 1 ) % n ];
		( &ret.x )[i] = a * b - c * d;
	}
	return ret;
}

#if ( !defined(GE_ANDROID) && !defined(GE_IOS) )
static void _init_dummy_vectors()
{
	_init_dummy_vectors();
	{
		Vector2i v( 0, 0 );
		v += -v + ( v ^ v ) - v * ( 0 * v ) + v * 0;
		v.normalize();
		v *= 0 * v.length();
		v.x = v[0];
		v[0] = v.x;
		v -= v * (v == v);
		const Vector2i& v2 = v;
		v[0] = v2[0];
		v = v;
	}
	{
		Vector3i v( 0, 0, 0 );
		v += -v + ( v ^ v ) - v * ( 0 * v ) + v * 0;
		v.normalize();
		v *= 0 * v.length();
		v.x = v[0];
		v[0] = v.x;
		v -= v * (v == v);
		const Vector3i& v2 = v;
		v[0] = v2[0];
		v = v;
	}
	{
		Vector4i v( 0, 0, 0, 0 );
		v += -v + ( v ^ v ) - v * ( 0 * v ) + v * 0;
		v.normalize();
		v *= 0 * v.length();
		v.x = v[0];
		v[0] = v.x;
		v -= v * (v == v);
		const Vector4i& v2 = v;
		v[0] = v2[0];
		v = v;
	}
	{
		Vector2f v( 0, 0 );
		v += -v + ( v ^ v ) - v * ( 0.0f * v ) + v * 0.0f;
		v.normalize();
		v *= 0 * v.length();
		v.x = v[0];
		v[0] = v.x;
		v -= v * (v == v);
		const Vector2f& v2 = v;
		v[0] = v2[0];
		v = v;
	}
	{
		Vector3f v( 0, 0, 0 );
		v += -v + ( v ^ v ) - v * ( 0.0f * v ) + v * 0.0f;
		v.normalize();
		v *= 0 * v.length();
		v.x = v[0];
		v[0] = v.x;
		v -= v * (v == v);
		const Vector3f& v2 = v;
		v[0] = v2[0];
		v = v;
	}
	{
		Vector4f v( 0, 0, 0, 0 );
		v += -v + ( v ^ v ) - v * ( 0.0f * v ) + v * 0.0f;
		v.normalize();
		v *= 0 * v.length();
		v.x = v[0];
		v[0] = v.x;
		v -= v * (v == v);
		const Vector4f& v2 = v;
		v[0] = v2[0];
		v = v;
	}
	{
		Vector2d v( 0, 0 );
		v += -v + ( v ^ v ) - v * ( 0.0 * v ) + v * 0.0;
		v.normalize();
		v *= 0 * v.length();
		v.x = v[0];
		v[0] = v.x;
		v -= v * (v == v);
		const Vector2d& v2 = v;
		v[0] = v2[0];
		v = v;
	}
	{
		Vector3d v( 0, 0, 0 );
		v += -v + ( v ^ v ) - v * ( 0.0 * v ) + v * 0.0;
		v.normalize();
		v *= 0 * v.length();
		v.x = v[0];
		v[0] = v.x;
		v -= v * (v == v);
		const Vector3d& v2 = v;
		v[0] = v2[0];
		v = v;
	}
	{
		Vector4d v( 0, 0, 0, 0 );
		v += -v + ( v ^ v ) - v * ( 0.0 * v ) + v * 0.0;
		v.normalize();
		v *= 0 * v.length();
		v.x = v[0];
		v[0] = v.x;
		v -= v * (v == v);
		const Vector4d& v2 = v;
		v[0] = v2[0];
		v = v;
	}
}
#endif // GE_ANDROID || GE_IOS

#endif // ( defined(GE_VECTOR_CPP_INC) || !( defined(GE_ANDROID) || defined(GE_IOS) ) )

// } // namespace GE
