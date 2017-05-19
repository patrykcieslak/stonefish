//
//  SeaEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SeaEntity__
#define __Stonefish_SeaEntity__

#include "FluidEntity.h"
#include "MaterialManager.h"

class SeaEntity : public FluidEntity
{
public:
    SeaEntity(std::string uniqueName, Fluid* fld, btScalar size = btScalar(1000));
    virtual ~SeaEntity();
    
    bool IsInsideFluid(const btVector3& point);
    btScalar GetPressure(const btVector3& point);
    
private:
};

#endif
