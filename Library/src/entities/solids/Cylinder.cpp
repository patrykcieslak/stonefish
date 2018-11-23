//
//  Cylinder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Cylinder.h"

#include "utils/MathUtil.hpp"

using namespace sf;

Cylinder::Cylinder(std::string uniqueName, btScalar cylinderRadius, btScalar cylinderHeight, const btTransform& originTrans, Material m, int lookId, btScalar thickness, bool isBuoyant) : SolidEntity(uniqueName, m, lookId, thickness, isBuoyant)
{
    radius = UnitSystem::SetLength(cylinderRadius);
    halfHeight = UnitSystem::SetLength(cylinderHeight/btScalar(2));
    localTransform = UnitSystem::SetTransform(originTrans);
    
    //Calculate physical properties
    if(thick > btScalar(0) && thick/btScalar(2) < radius && thick/btScalar(2) < halfHeight)
    {
        btScalar r1 = radius - thick/btScalar(2);
        btScalar r2 = radius + thick/btScalar(2);
        btScalar h1 = halfHeight - thick/btScalar(2);
        btScalar h2 = halfHeight + thick/btScalar(2);
        volume = M_PI*(r2*r2*h2 - r1*r1*h1)*btScalar(2);
        mass = volume * mat.density;
        btScalar m1 = M_PI*(r1*r1*h1)*btScalar(2)*mat.density;
        btScalar m2 = M_PI*(r2*r2*h2)*btScalar(2)*mat.density;
        btScalar Ia = m2*r2*r2/btScalar(2) - m1*r1*r1/btScalar(2); 
        btScalar Ip = m2*(btScalar(3)*r2*r2 + (h2*btScalar(2))*(h2*btScalar(2)))/btScalar(12) - m1*(btScalar(3)*r1*r1 + (h1*btScalar(2))*(h1*btScalar(2)))/btScalar(12);
        Ipri = btVector3(Ip,Ip,Ia);
    }
    else
    {
        volume = M_PI*radius*radius*halfHeight*btScalar(2);
        mass = volume * mat.density;
        btScalar Ia = mass*radius*radius/btScalar(2);
        btScalar Ip = mass*(btScalar(3)*radius*radius + (halfHeight*btScalar(2))*(halfHeight*btScalar(2)))/btScalar(12);
        Ipri = btVector3(Ip,Ip,Ia);
    }
    
    //Build geometry
    mesh = OpenGLContent::BuildCylinder(radius, halfHeight*(GLfloat)2);
    transformMesh(mesh, localTransform);
    
    //Compute hydrodynamic properties
    ComputeHydrodynamicProxy(HYDRO_PROXY_CYLINDER);
    CoB = localTransform.getOrigin();
    //dragCoeff = btVector3(radius*halfHeight*btScalar(4*0.5), M_PI*radius*radius*btScalar(0.9), radius*halfHeight*btScalar(4*0.5));
}

Cylinder::~Cylinder()
{
}

SolidType Cylinder::getSolidType()
{
    return SOLID_CYLINDER;
}

btCollisionShape* Cylinder::BuildCollisionShape()
{
    btCylinderShape* cylShape = new btCylinderShapeZ(btVector3(radius, radius, halfHeight));
    btCompoundShape* colShape = new btCompoundShape();
    colShape->addChildShape(localTransform, cylShape);
    //colShape->setMargin(0.0);
    return colShape;
}
