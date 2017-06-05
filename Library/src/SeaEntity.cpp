//
//  SeaEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright(c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "SeaEntity.h"

SeaEntity::SeaEntity(std::string uniqueName, Fluid* fld, btScalar size) : FluidEntity(uniqueName, fld)
{
    depth = UnitSystem::SetLength(size);
    
    btVector3 halfExtents = btVector3(UnitSystem::SetLength(size/btScalar(2)), UnitSystem::SetLength(size/btScalar(2)), size/btScalar(2));
    ghost->setWorldTransform(btTransform(btQuaternion::getIdentity(), btVector3(0,0,size/btScalar(2)))); //Surface at 0
    ghost->setCollisionShape(new btBoxShape(halfExtents));
    
    surfaceDisplayList = glGenLists(1);
    glNewList(surfaceDisplayList, GL_COMPILE);
    glColor4f(1.f,1.f,1.f,1.f);
    glNormal3f(0, 0, -1.f);
    glBegin(GL_QUADS);
    glVertex3f(-halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    glVertex3f(-halfExtents.x(), halfExtents.y(), -halfExtents.z());
    glVertex3f(halfExtents.x(), halfExtents.y(), -halfExtents.z());
    glVertex3f(halfExtents.x(), -halfExtents.y(), -halfExtents.z());
    glEnd();
    glEndList();
}

SeaEntity::~SeaEntity()
{
}

bool SeaEntity::IsInsideFluid(const btVector3& point) const
{
    return point.z() >= btScalar(0);
}

btScalar SeaEntity::GetPressure(const btVector3& point) const
{
    btScalar g = 9.81;
    btScalar d = point.z();
    btScalar pressure = d > btScalar(0) ? d*fluid->density*g : btScalar(0);
    return pressure;
}

