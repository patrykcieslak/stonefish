//
//  FluidEntity.cpp
//  StonefishConstructor
//
//  Created by Patryk Cieslak on 10/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "FluidEntity.h"

FluidEntity::FluidEntity(std::string uniqueName, Fluid* fld) : GhostEntity(uniqueName)
{
    surfaceDisplayList = 0;
    volumeDisplayList = 0;
    ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    fluid = fld;
}

FluidEntity::~FluidEntity()
{
    fluid = NULL;
    
    if(volumeDisplayList != 0)
        glDeleteLists(volumeDisplayList, 1);
    
    if(surfaceDisplayList != 0)
        glDeleteLists(surfaceDisplayList, 1);
}

GhostType FluidEntity::getGhostType()
{
    return FLUID;
}

void FluidEntity::GetSurface(btVector3& normal, btVector3& position) const
{
    normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
}

void FluidEntity::GetSurfaceEquation(double* plane4) const
{
    btVector3 normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    btVector3 position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
    plane4[0] = normal.x();
    plane4[1] = normal.y();
    plane4[2] = normal.z();
    plane4[3] = -normal.dot(position);
}

btScalar FluidEntity::getDepth()
{
    return depth;
}

const Fluid* FluidEntity::getFluid() const
{
    return fluid;
}

void FluidEntity::Render()
{
}

void FluidEntity::RenderSurface()
{
    btTransform trans = ghost->getWorldTransform();
    btScalar openglTrans[16];
    trans.getOpenGLMatrix(openglTrans);
    
    glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
    glMultMatrixd(openglTrans);
#else
    glMultMatrixf(openglTrans);
#endif
    glCallList(surfaceDisplayList);
    glPopMatrix();
}

void FluidEntity::RenderVolume()
{
    btTransform trans = ghost->getWorldTransform();
    btScalar openglTrans[16];
    trans.getOpenGLMatrix(openglTrans);
    
    glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
    glMultMatrixd(openglTrans);
#else
    glMultMatrixf(openglTrans);
#endif
    glCallList(volumeDisplayList);
    glPopMatrix();
}