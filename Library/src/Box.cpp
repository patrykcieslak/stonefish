//
//  Box.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "Box.h"

Box::Box(std::string uniqueName, const btVector3& dimensions, Material* mat, int lookId) : SolidEntity(uniqueName, mat, lookId)
{
    halfExtents = UnitSystem::SetPosition(dimensions * btScalar(0.5));
    
    //Calculate physical properties
    volume = halfExtents.x()*halfExtents.y()*halfExtents.z()*btScalar(8);
    dragCoeff = btVector3(halfExtents.y()*halfExtents.z()*btScalar(4*1.05), halfExtents.x()*halfExtents.z()*btScalar(4*1.05), halfExtents.y()*halfExtents.x()*btScalar(4*1.05));
    mass = volume * material->density;
    Ipri = btVector3(btScalar(1)/btScalar(12)*mass*((halfExtents.y()*btScalar(2))*(halfExtents.y()*btScalar(2))+(halfExtents.z()*btScalar(2))*(halfExtents.z()*btScalar(2))),
                     btScalar(1)/btScalar(12)*mass*((halfExtents.x()*btScalar(2))*(halfExtents.x()*btScalar(2))+(halfExtents.z()*btScalar(2))*(halfExtents.z()*btScalar(2))),
                     btScalar(1)/btScalar(12)*mass*((halfExtents.x()*btScalar(2))*(halfExtents.x()*btScalar(2))+(halfExtents.y()*btScalar(2))*(halfExtents.y()*btScalar(2))));
	
	glm::vec3 glHalfExtents(halfExtents.x(), halfExtents.y(), halfExtents.z());
	mesh = OpenGLContent::BuildBox(glHalfExtents);
}

Box::~Box()
{
}

SolidEntityType Box::getSolidType()
{
    return SOLID_BOX;
}

btCollisionShape* Box::BuildCollisionShape()
{
    btBoxShape* colShape = new btBoxShape(halfExtents);
    colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
    return colShape;
}

