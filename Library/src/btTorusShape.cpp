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

#include "btTorusShape.h"

btTorusShape::btTorusShape(btScalar majorRadius, btScalar minorRadius)
{
	m_majorRadius = majorRadius;
	m_minorRadius = minorRadius;
    
    setSafeMargin(btVector3(m_majorRadius + m_minorRadius, m_minorRadius, m_majorRadius + m_minorRadius));
	
	btVector3 margin(getMargin(),getMargin(),getMargin());
	m_implicitShapeDimensions = (btVector3(m_majorRadius + m_minorRadius, m_minorRadius, m_majorRadius + m_minorRadius) * m_localScaling) - margin;
	m_shapeType = CUSTOM_CONVEX_SHAPE_TYPE;
}

btVector3 btTorusShape::getHalfExtentsWithMargin() const
{
	btVector3 halfExtents = getHalfExtentsWithoutMargin();
	btVector3 margin(getMargin(),getMargin(),getMargin());
	halfExtents += margin;
	return halfExtents;
}

const btVector3& btTorusShape::getHalfExtentsWithoutMargin() const
{
	return m_implicitShapeDimensions;
}

void btTorusShape::getAabb(const btTransform& t,btVector3& aabbMin,btVector3& aabbMax) const
{
	btTransformAabb(getHalfExtentsWithoutMargin(),getMargin(),t,aabbMin,aabbMax);
}
	
void btTorusShape::calculateLocalInertia(btScalar mass,btVector3& inertia) const
{
	//Torus with Y principal axis
	btScalar idiam, ivert;
	idiam = btScalar(1.)/btScalar(8.)*(btScalar(4.)*m_majorRadius*m_majorRadius + btScalar(5.)*m_minorRadius*m_minorRadius)*mass;
	ivert = (m_majorRadius*m_majorRadius + btScalar(3.)/btScalar(4.)*m_minorRadius*m_minorRadius)*mass;
	inertia.setValue(ivert, idiam, ivert);
}

void btTorusShape::setLocalScaling(const btVector3& scaling)
{
    btScalar m_minorRadiusOld = m_minorRadius;
    m_minorRadius *= scaling[1] / m_localScaling[1];
    m_majorRadius = (m_majorRadius + m_minorRadiusOld) * scaling[0] / m_localScaling[0] - m_minorRadius;
    btConvexInternalShape::setLocalScaling(scaling);
}

btVector3 btTorusShape::localGetSupportingVertex(const btVector3& vec)const
{
	//Torus with Y principal axis
	btScalar s = btSqrt(vec[0]*vec[0] + vec[2]*vec[2]);
	btVector3 res;
	
	if(s != btScalar(0.0))
	{
		btScalar radial = m_majorRadius + m_minorRadius * s; //s => cos(alpha)
		res[0] = vec[0] * radial/s;
		res[1] = m_minorRadius * vec[1]; //vec[1] => sin(alpha)
		res[2] = vec[2] * radial/s;
	}
	else
	{
		res[0] = m_majorRadius;
		res[1] = vec[1] < 0.0 ? -m_minorRadius : m_minorRadius;
		res[2] = btScalar(0.0);
	}	 
	
	return res;
}

btVector3 btTorusShape::localGetSupportingVertexWithoutMargin(const btVector3& vec)const
{
	//Torus with Y principal axis
	btScalar s = btSqrt(vec[0]*vec[0] + vec[2]*vec[2]);
	btVector3 res;
	
	if(s != btScalar(0.0))
	{
		btScalar radial = m_majorRadius + (m_minorRadius - getMargin()) * s; //s => cos(alpha)
		res[0] = vec[0] * radial/s;
		res[1] = (m_minorRadius - getMargin()) * vec[1]; //vec[1] => sin(alpha)
		res[2] = vec[2] * radial/s;
	}
	else
	{
		res[0] = m_majorRadius;
		res[1] = vec[1] < 0.0 ? -(m_minorRadius - getMargin()) : (m_minorRadius - getMargin());
		res[2] = btScalar(0.0);
	}	 
	
	return res;
}

void btTorusShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const
{
	for (int i=0;i<numVectors;i++)
	{
        btVector3 support = localGetSupportingVertexWithoutMargin(vectors[i]);
		supportVerticesOut[i].setValue(support.x(), support.y(), support.z());
	}
}
