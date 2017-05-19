//
//  FluidEntity.cpp
//  StonefishConstructor
//
//  Created by Patryk Cieslak on 10/13/13.
//  Copyright(c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "FluidEntity.h"
#include "SolidEntity.h"

FluidEntity::FluidEntity(std::string uniqueName, Fluid* fld) : ForcefieldEntity(uniqueName)
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

ForcefieldType FluidEntity::getForcefieldType()
{
    return FORCEFIELD_FLUID;
}

btScalar FluidEntity::getDepth()
{
    return depth;
}

const Fluid* FluidEntity::getFluid() const
{
    return fluid;
}

btVector3 FluidEntity::GetFluidVelocity(const btVector3& point)
{
    return btVector3(0,0,0);
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

void FluidEntity::ApplyFluidForces(btDynamicsWorld* world, btCollisionObject* co)
{
    //Check if object is an Entity
    btRigidBody* rb = btRigidBody::upcast(co);
    if(rb == NULL || rb->isStaticObject())
        return;
    
    Entity* ent = (Entity*)rb->getUserPointer();
    if(ent == NULL) 
        return;
    
    if(ent->getType() == ENTITY_SOLID)
        ((SolidEntity*)ent)->ApplyFluidForces(this);
    
    /*
    //2.Determine fluid surface coordinates
    btVector3 surfaceN, surfaceD;
    GetSurface(surfaceN, surfaceD);
    
    //3.Calculate fluid forces and buoyancy center based on entity type
    btVector3 cob;
    btScalar submergedV = 0;
    btVector3 dForce;
    btVector3 adTorque;
    btVector3 fluidVelocity(0,0,0);
    
    switch (ent->getType())
    {
        case ENTITY_SOLID:
        {
            SolidEntity* solid = (SolidEntity*)ent;
            solid->CalculateFluidDynamics(surfaceN, surfaceD, fluidVelocity, fluid, submergedV, cob, dForce, adTorque);
            rb->applyForce(-world->getGravity()*submergedV*fluid->density, cob);
            rb->applyCentralForce(dForce);
            rb->applyTorque(adTorque);
            rb->setDamping(submergedV/solid->getVolume()*fluid->viscousity, submergedV/solid->getVolume()*fluid->viscousity);
        }
        break;
    
        case ENTITY_CABLE:
        {
            CableEntity* cable = (CableEntity*)ent;
            cable->CalculateFluidDynamics(surfaceN, surfaceD, fluidVelocity, fluid, submergedV, cob, dForce, adTorque, rb->getWorldTransform(), rb->getLinearVelocity(), rb->getAngularVelocity());
            rb->applyForce(-world->getGravity()*submergedV*fluid->density, cob);
            rb->applyCentralForce(dForce);
            rb->applyTorque(adTorque);
            rb->setDamping(submergedV/cable->getPartVolume()*fluid->viscousity, submergedV/cable->getPartVolume()*fluid->viscousity);
        }
            break;

        default:
            return;
    }*/
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