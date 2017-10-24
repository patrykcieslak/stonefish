//
//  Liquid.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Liquid.h"
#include "SolidEntity.h"
#include "SystemEntity.h"
#include "MathsUtil.hpp"
#include "SystemUtil.hpp"
#include <algorithm>

Liquid::Liquid(std::string uniqueName, Fluid* f) : ForcefieldEntity(uniqueName)
{
	ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    fluid = f;
    
	btScalar size(10000);
	depth = UnitSystem::SetLength(size);
    
    btVector3 halfExtents = btVector3(UnitSystem::SetLength(size/btScalar(2)), UnitSystem::SetLength(size/btScalar(2)), size/btScalar(2));
    ghost->setWorldTransform(btTransform(btQuaternion::getIdentity(), btVector3(0,0,size/btScalar(2)))); //Surface at 0
    ghost->setCollisionShape(new btBoxShape(halfExtents));
}

Liquid::~Liquid()
{
    fluid = NULL;
}

btScalar Liquid::GetDepth(const btVector3& point) const
{
    return point.z();
}

const Fluid* Liquid::getFluid() const
{
    return fluid;
}

bool Liquid::IsInsideFluid(const btVector3& point) const
{
    return point.z() >= btScalar(0);
}

btScalar Liquid::GetPressure(const btVector3& point) const
{
    btScalar g = 9.81;
    btScalar d = point.z();
    btScalar pressure = d > btScalar(0) ? d*fluid->density*g : btScalar(0);
    return pressure;
}

btVector3 Liquid::GetFluidVelocity(const btVector3& point) const
{
    return btVector3(0,0,0);
}

void Liquid::GetSurface(btVector3& normal, btVector3& position) const
{
    normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
}

void Liquid::GetSurfaceEquation(double* plane4) const
{
    btVector3 normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    btVector3 position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
    plane4[0] = normal.x();
    plane4[1] = normal.y();
    plane4[2] = normal.z();
    plane4[3] = -normal.dot(position);
}

void Liquid::ApplyFluidForces(const HydrodynamicsType ht, btDynamicsWorld* world, btCollisionObject* co, bool recompute)
{
    Entity* ent;
	btRigidBody* rb = btRigidBody::upcast(co);
	btMultiBodyLinkCollider* mbl = btMultiBodyLinkCollider::upcast(co);
	
	if(rb != 0)
	{
		if(rb->isStaticOrKinematicObject())
			return;
		else
			ent = (Entity*)rb->getUserPointer();
	}
	else if(mbl != 0)
	{
		if(mbl->isStaticOrKinematicObject())
			return;
		else
			ent = (Entity*)mbl->getUserPointer();
	}
	
	HydrodynamicsSettings settings;
	settings.algorithm = ht;
	settings.addedMassForces = true;
	settings.dampingForces = true;
	settings.reallisticBuoyancy = true;
	
    if(ent->getType() == ENTITY_SOLID)
    {
        if(recompute) 
            ((SolidEntity*)ent)->ComputeFluidForces(settings, this);
        
        ((SolidEntity*)ent)->ApplyFluidForces();
    }
}