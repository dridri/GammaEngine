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

#include "PhysicalCollider.h"
#include "PhysicalGraph.h"
#include "Instance.h"
#include "Debug.h"

using namespace GE;

PhysicalCollider::PhysicalCollider( PhysicalGraph* parent )
	: mParent( parent )
{
}


PhysicalCollider::~PhysicalCollider()
{
}

/*
extern float collide_x;
extern float collide_y;
extern float collide1_x;
extern float collide1_y;
extern float collide2_x;
extern float collide2_y;
*/
void PhysicalCollider::Collide( PhysicalBody* a, PhysicalBody* b )
{
// 	uint32_t nVerts = 0;
// 	Vector3f* verts = minkowskiDifference( a, b, &nVerts );

	std::cout << "\n";

	uint32_t nVertsA = 0;
	uint32_t nVertsB = 0;
	Vector3f* vertsA = a->collisionMesh( &nVertsA );
	Vector3f* vertsB = b->collisionMesh( &nVertsB );

	vertsA = (Vector3f*)mParent->instance()->Malloc( sizeof(Vector3f) * nVertsA );
	vertsB = (Vector3f*)mParent->instance()->Malloc( sizeof(Vector3f) * nVertsB );
	for ( uint32_t i = 0; i < nVertsA || i < nVertsB; i++ ) {
		if ( i < nVertsA ) {
			vertsA[i] = ( a->matrix() * Vector4f( a->collisionMesh()[i], 1.0f ) ).xyz();
		}
		if ( i < nVertsB ) {
			vertsB[i] = ( b->matrix() * Vector4f( b->collisionMesh()[i], 1.0f ) ).xyz();
		}
	}

	bool colliding = false;
	std::vector< Vector3f > simplex;
	Vector3f dir = Vector3f( 1.0f, 0.0f, 0.0f );
	simplex.emplace_back( std::get<0>( Support( vertsA, nVertsA, vertsB, nVertsB, dir ) ) );
	dir = -simplex.back();
	dir.normalize();

	std::tuple< Vector3f, uint32_t, uint32_t > lastSupport;

	int it = 0;
	while ( 1 ) {
		gDebug() << "it : " << it << "\n";
		lastSupport = Support( vertsA, nVertsA, vertsB, nVertsB, dir );
		simplex.emplace_back( std::get<0>( lastSupport ) );

		for ( uint32_t i=0; i<simplex.size(); i++ ) {
			vDebug( "simplex[" << i << "] = ", simplex.at(i).x, simplex.at(i).y, simplex.at(i).z ) << "\n";
		}

		if ( dir * simplex.back() <= 0.0f ) {
			gDebug() << "Dead (" << ( dir * simplex.back() ) << ")\n";
			colliding = false;
			break;
		}
		if ( simplex.size() == 2 ) {
			if ( ( -simplex[1] ) * ( simplex[0] - simplex[1] ) > 0.0f ) {
				dir = simplex[0] - simplex[1];
				dir = Vector3f( -dir.y, dir.x, dir.z );
				dir = dir * ( -simplex[1] * dir );
				Vector3f tmp = simplex[0];
				simplex[0] = simplex[1];
				simplex[1] = tmp;
			} else {
				dir = -simplex[1];
				simplex.erase( simplex.begin() );
			}
		} else if ( simplex.size() == 3 ) {
			Vector3f ab = simplex[1] - simplex[2];
			Vector3f ac = simplex[0] - simplex[2];
			Vector3f ao = -simplex[2];
			Vector3f acb = Vector3f( -ab.y, ab.x, ab.z );
			acb *= acb * (-ac);
			Vector3f abc = Vector3f( -ac.y, ac.x, ac.z );
			abc *= abc * (-ab);
			if ( acb * ao > 0.0f ) {
				if ( ab * ao > 0.0f ) {
					dir = acb;
					simplex.erase( simplex.begin() );
				} else {
					dir = ao;
					simplex.erase( simplex.begin() );
					simplex.erase( simplex.begin() );
				}
			} else if ( abc * ao > 0.0f ) {
				if ( ac * ao > 0.0f ) {
					dir = abc;
					simplex.erase( simplex.begin() + 1 );
				} else {
					dir = ao;
					simplex.erase( simplex.begin() );
					simplex.erase( simplex.begin() );
				}
			} else {
				gDebug() << "Colliding !\n";
				colliding = true;
				break;
			}
			dir.normalize();
		}
		it++;
	}

	if ( colliding ) {
		Vector3f ao = -std::get<0>( lastSupport );
		Vector3f bo = -std::get<0>( lastSupport );
		Vector3f collisionPoint_0 = vertsA[ std::get<1>( lastSupport ) ] + ao;
		Vector3f collisionPoint_1 = vertsB[ std::get<2>( lastSupport ) ] - bo;
		vDebug( "collisionPoint_0 = ", collisionPoint_0.x, collisionPoint_0.y, collisionPoint_0.z ) << "\n";
		vDebug( "vertsB[ std::get<2>( lastSupport ) ] = ", vertsB[ std::get<2>( lastSupport ) ].x, vertsB[ std::get<2>( lastSupport ) ].y, vertsB[ std::get<2>( lastSupport ) ].z ) << "\n";
		vDebug( "collisionPoint_1 = ", collisionPoint_1.x, collisionPoint_1.y, collisionPoint_1.z ) << "\n";
		vDebug( "vertsA[ std::get<1>( lastSupport ) ] = ", vertsA[ std::get<1>( lastSupport ) ].x, vertsA[ std::get<1>( lastSupport ) ].y, vertsA[ std::get<1>( lastSupport ) ].z ) << "\n";
		bool assertA = ( collisionPoint_0 == vertsB[ std::get<2>( lastSupport ) ] );
		bool assertB = ( collisionPoint_1 == vertsA[ std::get<1>( lastSupport ) ] );
		assertA = ( collisionPoint_0 * ao <= 0.0f );
		assertB = ( collisionPoint_1 * ao <= 0.0f );/*
		if ( assertA && !assertB ) {
			collide1_x = collisionPoint_0.x;
			collide1_y = collisionPoint_0.y;
			collide2_x = 0.0f;
			collide2_y = 0.0f;
			collide_x = collide1_x;
			collide_y = collide1_y;
		} else if ( assertB && !assertA ) {
			collide1_x = 0.0f;
			collide1_y = 0.0f;
			collide2_x = collisionPoint_1.x;
			collide2_y = collisionPoint_1.y;
			collide_x = collide2_x;
			collide_y = collide2_y;
		} else */{
			/*
			collide1_x = collisionPoint_0.x;
			collide1_y = collisionPoint_0.y;
			collide2_x = collisionPoint_1.x;
			collide2_y = collisionPoint_1.y;
			collide_x = ( collide1_x + collide2_x ) / 2.0f;
			collide_y = ( collide1_y + collide2_y ) / 2.0f;
			*/
		}
// 		a->HandleCollision( b, Vector3f( collide_x, collide_y, 0.0f ), dir );
	}

	mParent->instance()->Free( vertsA );
	mParent->instance()->Free( vertsB );
// 	mParent->instance()->Free( verts );
}


std::tuple< Vector3f, uint32_t, uint32_t > PhysicalCollider::Support( const Vector3f* A, uint32_t nVA, const Vector3f* B, uint32_t nVB, const Vector3f& dir )
{
	fDebug( A, nVA, B, nVB, dir.x, dir.y, dir.z );

	Vector3f a, b, r;
	float tmp = 0.0f;
	float fA = -1.0e33f;
	float fB = -1.0e33f;
	uint32_t iA = 0;
	uint32_t iB = 0;

	for ( uint32_t i = 0; i < nVA || i < nVB; i++ ) {
		if ( i < nVA && ( tmp = dir * A[i] ) > fA ) {
			fA = tmp;
			iA = i;
			a = A[i];
		}
		if ( i < nVB && ( tmp = (-dir) * B[i] ) > fB ) {
			fB = tmp;
			iB = i;
			b = B[i];
		}
	}
	vDebug( "a = ", a.x, a.y, a.z ) << "\n";
	vDebug( "b = ", b.x, b.y, b.z ) << "\n";
	r = a - b;
	vDebug( "r = ", r.x, r.y, r.z ) << "\n";
	gDebug() << "dir * r = " << ( dir * r ) << "\n";

	return std::make_tuple( r, iA, iB );
}


Vector3f* PhysicalCollider::minkowskiDifference( PhysicalBody* a, PhysicalBody* b, uint32_t* pnVerts )
{
	uint32_t i, j, k;
	uint32_t nVerts = 0;
	uint32_t nVertsA = 0;
	uint32_t nVertsB = 0;
	Vector3f* ret = nullptr;
	Vector3f* vertsA = a->collisionMesh( &nVertsA );
	Vector3f* vertsB = b->collisionMesh( &nVertsB );
	Vector3f vA, vB;

	nVerts = nVertsA * nVertsB;
	ret = ( Vector3f* )mParent->instance()->Malloc( sizeof(Vector3f) * nVerts );

	for ( j = 0, k = 0; j < nVertsA; j++ ) {
		for ( i = 0; i < nVertsB; i++, k++ ) {
			vA = ( a->matrix() * Vector4f( vertsA[j], 1.0f ) ).xyz();
			vB = ( b->matrix() * Vector4f( vertsB[i], 1.0f ) ).xyz();
			ret[k] = vA - vB;
		}
	}

	*pnVerts = nVerts;
	return ret;
}
