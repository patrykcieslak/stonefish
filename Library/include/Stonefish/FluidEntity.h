//
//  FluidEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FluidEntity__
#define __Stonefish_FluidEntity__

#include "GhostEntity.h"
#include "MaterialManager.h"

class FluidEntity : public GhostEntity
{
public:
    FluidEntity(std::string uniqueName, Fluid* fld);
    virtual ~FluidEntity();
    
    GhostType getGhostType();
    virtual void Render();
    virtual void RenderSurface();
    virtual void RenderVolume();
    virtual void ApplyFluidForces(btDynamicsWorld* world, btCollisionObject* co) = 0;
    virtual bool IsInsideFluid(const btVector3& point) = 0;
    
    void GetSurface(btVector3& normal, btVector3& position) const;
    void GetSurfaceEquation(double* plane4) const;
    btScalar getDepth();
    const Fluid* getFluid() const;
    
protected:
    Fluid* fluid;
    btScalar depth;
    GLint surfaceDisplayList;
    GLint volumeDisplayList;
};

#endif