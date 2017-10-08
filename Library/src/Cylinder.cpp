//
//  Cylinder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "Cylinder.h"

Cylinder::Cylinder(std::string uniqueName, btScalar cylinderRadius, btScalar cylinderHeight, Material m, int lookId) : SolidEntity(uniqueName, m, lookId)
{
    radius = UnitSystem::SetLength(cylinderRadius);
    halfHeight = UnitSystem::SetLength(cylinderHeight/btScalar(2));
    
    //Calculate physical properties
    //dragCoeff = btVector3(radius*halfHeight*btScalar(4*0.5), M_PI*radius*radius*btScalar(0.9), radius*halfHeight*btScalar(4*0.5));
    volume = M_PI*radius*radius*halfHeight*btScalar(2);
    mass = volume * mat.density;
    Ipri = btVector3(btScalar(0.25)*mass*radius*radius + btScalar(1)/btScalar(12)*mass*(halfHeight*btScalar(2))*(halfHeight*btScalar(2)),
                     btScalar(0.5)*mass*radius*radius,
                     btScalar(0.25)*mass*radius*radius + btScalar(1)/btScalar(12)*mass*(halfHeight*btScalar(2))*(halfHeight*btScalar(2)));
    
	mesh = OpenGLContent::BuildCylinder(radius, halfHeight*(GLfloat)2);
	ComputeEquivEllipsoid();
}

Cylinder::~Cylinder()
{
}

SolidEntityType Cylinder::getSolidType()
{
    return SOLID_CYLINDER;
}

btCollisionShape* Cylinder::BuildCollisionShape()
{
    btCylinderShape* colShape = new btCylinderShape(btVector3(radius, halfHeight, radius));
    colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
    return colShape;
}
