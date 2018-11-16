//
//  TorusShape.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/15/14.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "core/TorusShape.h"

TorusShape::TorusShape(btScalar majorRadius, btScalar minorRadius)
{
	m_majorRadius = majorRadius;
	m_minorRadius = minorRadius;
    
    setSafeMargin(btVector3(m_majorRadius + m_minorRadius, m_minorRadius, m_majorRadius + m_minorRadius));
	
	btVector3 margin(getMargin(),getMargin(),getMargin());
	m_implicitShapeDimensions = (btVector3(m_majorRadius + m_minorRadius, m_minorRadius, m_majorRadius + m_minorRadius) * m_localScaling) - margin;
	m_shapeType = CUSTOM_CONVEX_SHAPE_TYPE;
}

btVector3 TorusShape::getHalfExtentsWithMargin() const
{
	btVector3 halfExtents = getHalfExtentsWithoutMargin();
	btVector3 margin(getMargin(),getMargin(),getMargin());
	halfExtents += margin;
	return halfExtents;
}

const btVector3& TorusShape::getHalfExtentsWithoutMargin() const
{
	return m_implicitShapeDimensions;
}

void TorusShape::getAabb(const btTransform& t, btVector3& aabbMin, btVector3& aabbMax) const
{
	btTransformAabb(getHalfExtentsWithoutMargin(), getMargin(), t, aabbMin, aabbMax);
}
	
void TorusShape::calculateLocalInertia(btScalar mass, btVector3& inertia) const
{
	//Torus with Y principal axis
	btScalar idiam, ivert;
	idiam = btScalar(1.)/btScalar(8.)*(btScalar(4.)*m_majorRadius*m_majorRadius + btScalar(5.)*m_minorRadius*m_minorRadius)*mass;
	ivert = (m_majorRadius*m_majorRadius + btScalar(3.)/btScalar(4.)*m_minorRadius*m_minorRadius)*mass;
	inertia.setValue(ivert, idiam, ivert);
}

void TorusShape::setLocalScaling(const btVector3& scaling)
{
    btScalar m_minorRadiusOld = m_minorRadius;
    m_minorRadius *= scaling[1] / m_localScaling[1];
    m_majorRadius = (m_majorRadius + m_minorRadiusOld) * scaling[0] / m_localScaling[0] - m_minorRadius;
    btConvexInternalShape::setLocalScaling(scaling);
}

btVector3 TorusShape::localGetSupportingVertex(const btVector3& vec)const
{
	//Torus with Y principal axis
	btScalar s = btSqrt(vec[0]*vec[0] + vec[2]*vec[2]);
	btVector3 res;
	
	if(!btFuzzyZero(s))
	{
		btScalar radial = m_majorRadius + m_minorRadius * s; //s => cos(alpha)
		res[0] = vec[0]/s * radial;
		res[1] = m_minorRadius * vec[1]; //vec[1] => sin(alpha)
		res[2] = vec[2]/s * radial;
	}
	else
	{
		res[0] = m_majorRadius;
		res[1] = vec[1] < 0.0 ? -m_minorRadius : m_minorRadius;
		res[2] = btScalar(0.0);
	}	 
	
	return res;
}

btVector3 TorusShape::localGetSupportingVertexWithoutMargin(const btVector3& vec)const
{
	//Torus with Y principal axis
	btScalar s = btSqrt(vec[0]*vec[0] + vec[2]*vec[2]);
	btVector3 res;
	
	if(!btFuzzyZero(s))
	{
		btScalar radial = m_majorRadius + (m_minorRadius - getMargin()) * s; //s => cos(alpha)
		res[0] = vec[0]/s * radial;
		res[1] = (m_minorRadius - getMargin()) * vec[1]; //vec[1] => sin(alpha)
		res[2] = vec[2]/s * radial;
	}
	else
	{
		res[0] = m_majorRadius;
		res[1] = vec[1] < 0.0 ? -(m_minorRadius - getMargin()) : (m_minorRadius - getMargin());
		res[2] = btScalar(0.0);
	}	 
	
	return res;
}

void TorusShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors, btVector3* supportVerticesOut, int numVectors) const
{
	for (int i=0;i<numVectors;i++)
	{
        btVector3 support = localGetSupportingVertexWithoutMargin(vectors[i]);
		supportVerticesOut[i].setValue(support.x(), support.y(), support.z());
	}
}
