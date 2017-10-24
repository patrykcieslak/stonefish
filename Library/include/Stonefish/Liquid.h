//
//  Liquid.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Liquid__
#define __Stonefish_Liquid__

#include "ForcefieldEntity.h"
#include "MaterialManager.h"
#include "OpenGLPool.h"

typedef enum {TRIFOLD_SYMMETRY, FULLY_COUPLED, GEOMETRY_BASED} HydrodynamicsType;

typedef struct
{
	HydrodynamicsType algorithm;
	bool addedMassForces;
	bool dampingForces;
	bool reallisticBuoyancy;
} HydrodynamicsSettings;

//Abstract class
class Liquid : public ForcefieldEntity
{
public:
    Liquid(std::string uniqueName, Fluid* f);
    virtual ~Liquid();
    
    //Forces
    virtual void ApplyFluidForces(const HydrodynamicsType ht, btDynamicsWorld* world, btCollisionObject* co, bool recompute);
    virtual btVector3 GetFluidVelocity(const btVector3& point) const;
    void GetSurface(btVector3& normal, btVector3& position) const;
    void GetSurfaceEquation(double* plane4) const;
    
    //Getters
    OpenGLPool* getOpenGLPool();
    const Fluid* getFluid() const;
    virtual ForcefieldType getForcefieldType() = 0;
	virtual bool IsInsideFluid(const btVector3& point) const;
    virtual btScalar GetPressure(const btVector3& point) const;
    virtual btScalar GetDepth(const btVector3& point) const;
	
private:	
    Fluid* fluid;
    btScalar depth;
    OpenGLPool* glPool;
};
#endif
