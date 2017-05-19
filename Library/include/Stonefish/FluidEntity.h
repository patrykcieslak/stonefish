//
//  FluidEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/13/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FluidEntity__
#define __Stonefish_FluidEntity__

#include "ForcefieldEntity.h"
#include "MaterialManager.h"

//Abstract class
class FluidEntity : public ForcefieldEntity
{
public:
    FluidEntity(std::string uniqueName, Fluid* fld);
    virtual ~FluidEntity();
    
    //Forces
    virtual void ApplyFluidForces(btDynamicsWorld* world, btCollisionObject* co);
    virtual btVector3 GetFluidVelocity(const btVector3& point);
    void GetSurface(btVector3& normal, btVector3& position) const;
    void GetSurfaceEquation(double* plane4) const;
    
    //Display
    virtual void Render();
    virtual void RenderSurface();
    virtual void RenderVolume();
    
    //Getters
    btScalar getDepth();
    const Fluid* getFluid() const;
    ForcefieldType getForcefieldType();
    
    //Abstract
    virtual bool IsInsideFluid(const btVector3& point) = 0;
    virtual btScalar GetPressure(const btVector3& point) = 0;
    
protected:
    Fluid* fluid;
    btScalar depth;
    GLint surfaceDisplayList;
    GLint volumeDisplayList;
};

#endif