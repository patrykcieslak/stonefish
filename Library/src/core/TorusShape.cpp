/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  TorusShape.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/15/14.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "core/TorusShape.h"

namespace sf
{

TorusShape::TorusShape(Scalar majorRadius, Scalar minorRadius)
{
    majorRadius_ = majorRadius;
    minorRadius_ = minorRadius;
    
    setSafeMargin(Vector3(majorRadius_ + minorRadius_, minorRadius_, majorRadius_ + minorRadius_));
    
    Vector3 margin(getMargin(),getMargin(),getMargin());
    m_implicitShapeDimensions = (Vector3(majorRadius_ + minorRadius_, minorRadius_, majorRadius_ + minorRadius_) * m_localScaling) - margin;
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
    idiam = Scalar(1.)/Scalar(8.)*(Scalar(4.)*majorRadius_*majorRadius_ + Scalar(5.)*minorRadius_*minorRadius_)*mass;
    ivert = (majorRadius_*majorRadius_ + Scalar(3.)/Scalar(4.)*minorRadius_*minorRadius_)*mass;
    inertia.setValue(ivert, idiam, ivert);
}

void TorusShape::setLocalScaling(const Vector3& scaling)
{
    Scalar m_minorRadiusOld = minorRadius_;
    minorRadius_ *= scaling[1] / m_localScaling[1];
    majorRadius_ = (majorRadius_ + m_minorRadiusOld) * scaling[0] / m_localScaling[0] - minorRadius_;
    btConvexInternalShape::setLocalScaling(scaling);
}

Vector3 TorusShape::localGetSupportingVertex(const Vector3& vec)const
{
    //Torus with Y principal axis
    Scalar s = btSqrt(vec[0]*vec[0] + vec[2]*vec[2]);
    Vector3 res;
    
    if(!btFuzzyZero(s))
    {
        Scalar radial = majorRadius_ + minorRadius_ * s; //s => cos(alpha)
        res[0] = vec[0]/s * radial;
        res[1] = minorRadius_ * vec[1]; //vec[1] => sin(alpha)
        res[2] = vec[2]/s * radial;
    }
    else
    {
        res[0] = majorRadius_;
        res[1] = vec[1] < 0.0 ? -minorRadius_ : minorRadius_;
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
        Scalar radial = majorRadius_ + (minorRadius_ - getMargin()) * s; //s => cos(alpha)
        res[0] = vec[0]/s * radial;
        res[1] = (minorRadius_ - getMargin()) * vec[1]; //vec[1] => sin(alpha)
        res[2] = vec[2]/s * radial;
    }
    else
    {
        res[0] = majorRadius_;
        res[1] = vec[1] < 0.0 ? -(minorRadius_ - getMargin()) : (minorRadius_ - getMargin());
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

}
