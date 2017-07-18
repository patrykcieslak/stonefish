//
//  Ocean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/13/13.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Ocean__
#define __Stonefish_Ocean__

#include "ForcefieldEntity.h"
#include "MaterialManager.h"

//Abstract class
class Ocean : public ForcefieldEntity
{
public:
    Ocean(std::string uniqueName, Fluid* f);
    virtual ~Ocean();
    
    //Forces
    virtual void ApplyFluidForces(btDynamicsWorld* world, btCollisionObject* co);
    virtual btVector3 GetFluidVelocity(const btVector3& point) const;
    void GetSurface(btVector3& normal, btVector3& position) const;
    void GetSurfaceEquation(double* plane4) const;
    
    //Getters
    btScalar getDepth();
    const Fluid* getFluid() const;
    ForcefieldType getForcefieldType();
	QuadTree& getSurfaceMesh();
    
    virtual bool IsInsideFluid(const btVector3& point) const;
    virtual btScalar GetPressure(const btVector3& point) const;
    
protected:
    Fluid* fluid;
    btScalar depth;
    QuadTree surfaceMesh;
};
#endif
