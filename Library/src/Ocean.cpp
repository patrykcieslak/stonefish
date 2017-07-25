//
//  Ocean.cpp
//  StonefishConstructor
//
//  Created by Patryk Cieslak on 10/13/13.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Ocean.h"
#include "SolidEntity.h"
#include "SystemEntity.h"
#include "MathsUtil.hpp"
#include "SystemUtil.hpp"
#include <algorithm>

Ocean::Ocean(std::string uniqueName, Fluid* f) : ForcefieldEntity(uniqueName)
{
	ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    fluid = f;
	
	btScalar size(10000.);
	depth = UnitSystem::SetLength(size);
    
    btVector3 halfExtents = btVector3(UnitSystem::SetLength(size/btScalar(2)), UnitSystem::SetLength(size/btScalar(2)), size/btScalar(2));
    ghost->setWorldTransform(btTransform(btQuaternion::getIdentity(), btVector3(0,0,size/btScalar(2)))); //Surface at 0
    ghost->setCollisionShape(new btBoxShape(halfExtents));
}

Ocean::~Ocean()
{
    fluid = NULL;
}

ForcefieldType Ocean::getForcefieldType()
{
    return FORCEFIELD_FLUID;
}

btScalar Ocean::getDepth()
{
    return depth;
}

const Fluid* Ocean::getFluid() const
{
    return fluid;
}

OpenGLOcean& Ocean::getOpenGLOcean()
{
	return glOcean;
}

bool Ocean::IsInsideFluid(const btVector3& point) const
{
    return point.z() >= btScalar(0);
}

btScalar Ocean::GetPressure(const btVector3& point) const
{
    btScalar g = 9.81;
    btScalar d = point.z();
    btScalar pressure = d > btScalar(0) ? d*fluid->density*g : btScalar(0);
    return pressure;
}

btVector3 Ocean::GetFluidVelocity(const btVector3& point) const
{
    return btVector3(0,0,0);
}

void Ocean::GetSurface(btVector3& normal, btVector3& position) const
{
    normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
}

void Ocean::GetSurfaceEquation(double* plane4) const
{
    btVector3 normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    btVector3 position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
    plane4[0] = normal.x();
    plane4[1] = normal.y();
    plane4[2] = normal.z();
    plane4[3] = -normal.dot(position);
}

void Ocean::ApplyFluidForces(btDynamicsWorld* world, btCollisionObject* co)
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
    else if(ent->getType() == ENTITY_SYSTEM)
        ((SystemEntity*)ent)->ApplyFluidForces(this);
    
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