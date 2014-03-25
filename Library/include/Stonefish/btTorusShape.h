/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2014 Patryk Cieslak  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_TORUS_SHAPE_H
#define BT_TORUS_SHAPE_H

#include <BulletCollision/CollisionShapes/btConvexInternalShape.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseProxy.h> // for the types

ATTRIBUTE_ALIGNED16(class) btTorusShape : public btConvexInternalShape
{
	btScalar m_majorRadius;
	btScalar m_minorRadius;

public:
	BT_DECLARE_ALIGNED_ALLOCATOR();

	btTorusShape(btScalar majorRadius, btScalar minorRadius);

	btVector3 getHalfExtentsWithMargin() const;
	const btVector3& getHalfExtentsWithoutMargin() const;		
	virtual void getAabb(const btTransform& t,btVector3& aabbMin,btVector3& aabbMax) const;
	
	virtual void	calculateLocalInertia(btScalar mass,btVector3& inertia) const;

	virtual void setLocalScaling(const btVector3& scaling);

	virtual btVector3	localGetSupportingVertex(const btVector3& vec)const;
	virtual btVector3	localGetSupportingVertexWithoutMargin(const btVector3& vec)const;
	virtual void	batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const;

	btScalar getMajorRadius() const { return m_majorRadius; }
	btScalar getMinorRadius() const { return m_minorRadius; }
	btScalar getRadius() const { return getMajorRadius(); }
	
	
	virtual const char*	getName()const 
	{ 
		return "Torus"; 
	}
	
	virtual btVector3	getAnisotropicRollingFrictionDirection() const
	{
		return btVector3(0,1,1);
	}
};

#endif