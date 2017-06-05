//
//  PoolEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright(c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PoolEntity__
#define __Stonefish_PoolEntity__

#include "FluidEntity.h"
#include "MaterialManager.h"
#include "OpenGLMaterial.h"

#define CIRCULAR_TANK_DIV   24

class PoolEntity : public FluidEntity
{
public:
    PoolEntity(std::string uniqueName, btScalar extent1, btScalar extent2, btScalar depth, const btTransform& worldTransform, Fluid* fld, Material* wallMat, Look wallLook);
    PoolEntity(std::string uniqueName, btScalar radius, btScalar depth, const btTransform& worldTransform, Fluid* fld);
    virtual ~PoolEntity();
    
    void Render();
    bool IsInsideFluid(const btVector3& point) const;
    btScalar GetPressure(const btVector3& point) const;
    
private:
    //Physical
    Material* material;

    //Display
    Look look;
};

#endif
