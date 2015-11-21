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

#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>

namespace GE {

template <typename T, int n> class Vector
{
public:
	Vector( T x = 0, T y = 0, T z = 0, T w = 0 ) : x( x ), y( y ), z( z ), w( w ) {}
	Vector( const Vector<T,1>& v, T a = 0, T b = 0, T c = 0 ) : x(v.x), y(a), z(b), w(c) {}
	Vector( T a, const Vector<T,1>& v, T b = 0, T c = 0 ) : x(a), y(v.x), z(b), w(c) {}
	Vector( T a, T b, const Vector<T,1>& v, T c = 0 ) : x(a), y(b), z(v.x), w(c) {}
	Vector( T a, T b, T c, const Vector<T,1>& v ) : x(a), y(b), z(c), w(v.x) {}
	Vector( const Vector<T,2>& v, T a = 0, T b = 0 ) : x(v.x), y(v.z), z(a), w(b) {}
	Vector( T a, const Vector<T,2>& v, T b = 0 ) : x(a), y(v.x), z(v.y), w(b) {}
	Vector( T a, T b, const Vector<T,2>& v ) : x(a), y(b), z(v.x), w(v.y) {}
	Vector( const Vector<T,3>& v, T a = 0 ) : x(v.x), y(v.y), z(v.z), w(a) {}
	Vector(T a, const Vector<T,3>& v ) : x(a), y(v.x), z(v.y), w(v.z) {}
	Vector( const Vector<T,4>& v ) : x(v.x), y(v.y), z(v.z), w(v.w) {}
	Vector( float* v ) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

	Vector<T,n>& operator=( const Vector<T,n>& other );

	void normalize();
	T length();
	Vector<T,3> xyz() const { return Vector<T,3>( x, y, z ); }
	Vector<T,3> zyx() const { return Vector<T,3>( z, y, x ); }
	Vector<T,2> xy() const { return Vector<T,2>( x, y ); }
	Vector<T,2> xz() const { return Vector<T,2>( x, z ); }
	Vector<T,2> yz() const { return Vector<T,2>( y, z ); }

	T operator[]( int i ) const;
	T& operator[]( int i );
	Vector<T,n> operator-() const;
 	void operator+=( const Vector<T,n>& v );
 	void operator-=( const Vector<T,n>& v );
	void operator*=( T v );

	Vector<T,n> operator+( const Vector<T,n>& v ) const;
	Vector<T,n> operator-( const Vector<T,n>& v ) const;
	Vector<T,n> operator*( T im ) const;
	T operator*( const Vector<T,n>& v ) const;
	Vector<T,n> operator^( const Vector<T,n>& v ) const;

public:
	T x;
	T y;
	T z;
	T w;
} __attribute__((packed));

// template <typename T, int n> Vector<T, n> operator*( T im, const Vector<T, n>& v );
// template <typename T, int n> bool operator==( const Vector<T,n>& v1, const Vector<T,n>& v2 );

template <typename T, int n> Vector<T, n> operator*( T im, const Vector<T, n>& v ) {
	Vector<T, n> ret;
	for ( int i = 0; i < n; i++ ) {
		( &ret.x )[i] = ( &v.x )[i] * im;
	}
	return ret;
}


template <typename T, int n> bool operator==( const Vector<T, n>& v1, const Vector<T,n>& v2 ) {
	bool ret = true;
	for ( int i = 0; i < n; i++ ) {
		ret = ret && ( ( &v1.x )[i] == ( &v2.x )[i] );
	}
	return ret;
}


template <typename T, int n> bool operator!=( const Vector<T, n>& v1, const Vector<T,n>& v2 ) {
	bool ret = true;
	for ( int i = 0; i < n; i++ ) {
		ret = ret && ( ( &v1.x )[i] != ( &v2.x )[i] );
	}
	return ret;
}


typedef Vector<int, 2> Vector2i;
typedef Vector<int, 3> Vector3i;
typedef Vector<int, 4> Vector4i;

typedef Vector<float, 2> Vector2f;
typedef Vector<float, 3> Vector3f;
typedef Vector<float, 4> Vector4f;

typedef Vector<double, 2> Vector2d;
typedef Vector<double, 3> Vector3d;
typedef Vector<double, 4> Vector4d;


} // namespace GE

#if ( (defined(GE_ANDROID) || defined(GE_IOS) || defined(GE_RELEASE) )/* && defined(GE_LIB)*/ )
#define GE_VECTOR_CPP_INC
#ifdef GE_LIB
#include "../src/Vector.cpp"
#else
#include "Vector.cpp"
#endif
#endif

#endif // VECTOR_H
