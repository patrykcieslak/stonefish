//
//  TorusShape.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/15/14.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "core/TorusShape.h"

using namespace sf;

TorusShape::TorusShape(Scalar majorRadius, Scalar minorRadius)
{
	m_majorRadius = majorRadius;
	m_minorRadius = minorRadius;
    
    setSafeMargin(Vector3(m_majorRadius + m_minorRadius, m_minorRadius, m_majorRadius + m_minorRadius));
	
	Vector3 margin(getMargin(),getMargin(),getMargin());
	m_implicitShapeDimensions = (Vector3(m_majorRadius + m_minorRadius, m_minorRadius, m_majorRadius + m_minorRadius) * m_localScaling) - margin;
	m_shapeType = CUSTOM_CONVEX_SHAPE_TYPE;
}

Vector3 TorusShape::getHalfExtentsWithMargin() const
{
	Vector3 halfExtents = getHalfExtentsWithoutMargin();
	Vector3 margin(getMargin(),getMargin(),getMargin());
	halfExtents += margin;
	return halfExtents;
}

const Vector3& TorusShape::getHalfExtentsWithoutMargin() const
{
	return m_implicitShapeDimensions;
}

void TorusShape::getAabb(const Transform& t, Vector3& aabbMin, Vector3& aabbMax) const
{
	btTransformAabb(getHalfExtentsWithoutMargin(), getMargin(), t, aabbMin, aabbMax);
}
	
void TorusShape::calculateLocalInertia(Scalar mass, Vector3& inertia) const
{
	//Torus with Y principal axis
	Scalar idiam, ivert;
	idiam = Scalar(1.)/Scalar(8.)*(Scalar(4.)*m_majorRadius*m_majorRadius + Scalar(5.)*m_minorRadius*m_minorRadius)*mass;
	ivert = (m_majorRadius*m_majorRadius + Scalar(3.)/Scalar(4.)*m_minorRadius*m_minorRadius)*mass;
	inertia.setValue(ivert, idiam, ivert);
}

void TorusShape::setLocalScaling(const Vector3& scaling)
{
    Scalar m_minorRadiusOld = m_minorRadius;
    m_minorRadius *= scaling[1] / m_localScaling[1];
    m_majorRadius = (m_majorRadius + m_minorRadiusOld) * scaling[0] / m_localScaling[0] - m_minorRadius;
    btConvexInternalShape::setLocalScaling(scaling);
}

Vector3 TorusShape::localGetSupportingVertex(const Vector3& vec)const
{
	//Torus with Y principal axis
	Scalar s = btSqrt(vec[0]*vec[0] + vec[2]*vec[2]);
	Vector3 res;
	
	if(!btFuzzyZero(s))
	{
		Scalar radial = m_majorRadius + m_minorRadius * s; //s => cos(alpha)
		res[0] = vec[0]/s * radial;
		res[1] = m_minorRadius * vec[1]; //vec[1] => sin(alpha)
		res[2] = vec[2]/s * radial;
	}
	else
	{
		res[0] = m_majorRadius;
		res[1] = vec[1] < 0.0 ? -m_minorRadius : m_minorRadius;
		res[2] = Scalar(0.0);
	}	 
	
	return res;
}

Vector3 TorusShape::localGetSupportingVertexWithoutMargin(const Vector3& vec)const
{
	//Torus with Y principal axis
	Scalar s = btSqrt(vec[0]*vec[0] + vec[2]*vec[2]);
	Vector3 res;
	
	if(!btFuzzyZero(s))
	{
		Scalar radial = m_majorRadius + (m_minorRadius - getMargin()) * s; //s => cos(alpha)
		res[0] = vec[0]/s * radial;
		res[1] = (m_minorRadius - getMargin()) * vec[1]; //vec[1] => sin(alpha)
		res[2] = vec[2]/s * radial;
	}
	else
	{
		res[0] = m_majorRadius;
		res[1] = vec[1] < 0.0 ? -(m_minorRadius - getMargin()) : (m_minorRadius - getMargin());
		res[2] = Scalar(0.0);
	}	 
	
	return res;
}

void TorusShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vector3* vectors, Vector3* supportVerticesOut, int numVectors) const
{
	for (int i=0;i<numVectors;i++)
	{
        Vector3 support = localGetSupportingVertexWithoutMargin(vectors[i]);
		supportVerticesOut[i].setValue(support.x(), support.y(), support.z());
	}
}
