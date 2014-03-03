//
//  PoolEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PoolEntity__
#define __Stonefish_PoolEntity__

#include "FluidEntity.h"
#include "MaterialManager.h"

#define CIRCULAR_TANK_DIV   24

class PoolEntity : public FluidEntity
{
public:
    PoolEntity(std::string uniqueName, btScalar extent1, btScalar extent2, btScalar depth, const btTransform& worldTransform, Fluid* fld);
    PoolEntity(std::string uniqueName, btScalar radius, btScalar depth, const btTransform& worldTransform, Fluid* fld);
    virtual ~PoolEntity();
    
    void ApplyFluidForces(btDynamicsWorld* world, btCollisionObject* co);
    bool IsInsideFluid(const btVector3& point);
    
private:
    btVector3 fluidVelocity;
};

#endif
