//
//  TorusShape.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/15/14.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_TorusShape__
#define __Stonefish_TorusShape__

#include <BulletCollision/CollisionShapes/btConvexInternalShape.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseProxy.h>

ATTRIBUTE_ALIGNED16(class) TorusShape : public btConvexInternalShape
{
protected:
    btScalar m_majorRadius;
	btScalar m_minorRadius;
    
public:
	BT_DECLARE_ALIGNED_ALLOCATOR();

	TorusShape(btScalar majorRadius, btScalar minorRadius);

    virtual btVector3 localGetSupportingVertex(const btVector3& vec)const;
	virtual btVector3 localGetSupportingVertexWithoutMargin(const btVector3& vec)const;
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const;

	virtual void calculateLocalInertia(btScalar mass,btVector3& inertia) const;
    
	virtual void getAabb(const btTransform& t,btVector3& aabbMin,btVector3& aabbMax) const;
    
    btScalar getMajorRadius() const { return m_majorRadius; }
	btScalar getMinorRadius() const { return m_minorRadius; }
	btScalar getRadius() const { return getMajorRadius(); }
	
	btVector3 getHalfExtentsWithMargin() const;
	const btVector3& getHalfExtentsWithoutMargin() const;
	virtual void setLocalScaling(const btVector3& scaling);

	virtual const char*	getName()const { return "TORUS"; }
	
	virtual btVector3 getAnisotropicRollingFrictionDirection() const { return btVector3(0,1,0); }
};

#endif
