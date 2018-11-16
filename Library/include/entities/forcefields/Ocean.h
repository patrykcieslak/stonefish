//
//  Ocean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Ocean__
#define __Stonefish_Ocean__

#include "core/MaterialManager.h"
#include "graphics/OpenGLOcean.h"
#include "entities/ForcefieldEntity.h"
#include "entities/forcefields/VelocityField.h"

typedef enum {TRIFOLD_SYMMETRY, FULLY_COUPLED, GEOMETRY_BASED} HydrodynamicsType;

typedef struct
{
	HydrodynamicsType algorithm;
	bool addedMassForces;
	bool dampingForces;
	bool reallisticBuoyancy;
} HydrodynamicsSettings;

class Ocean : public ForcefieldEntity
{
public:
    Ocean(std::string uniqueName, Liquid* l);
    virtual ~Ocean();
    
    //Forces
    void AddVelocityField(VelocityField* field);
    virtual void ApplyFluidForces(const HydrodynamicsType ht, btDynamicsWorld* world, btCollisionObject* co, bool recompute);
    virtual btVector3 GetFluidVelocity(const btVector3& point) const;
    void GetSurface(btVector3& normal, btVector3& position) const;
    void GetSurfaceEquation(double* plane4) const;
    bool IsInsideFluid(const btVector3& point) const;
    btScalar GetPressure(const btVector3& point) const;
    btScalar GetDepth(const btVector3& point) const;
    glm::vec3 ComputeLightAbsorption();
    
    //Getters
    void setUseTrueWaves(bool waves);
    bool usesTrueWaves();
    void setAlgeaBloomFactor(GLfloat f);
    void setTurbidity(GLfloat ntu);
    GLfloat getAlgeaBloomFactor();
    GLfloat getTurbidity();
    const Liquid* getLiquid() const;
    OpenGLOcean* getOpenGLOcean();
    ForcefieldType getForcefieldType();
    
	void InitGraphics();
    std::vector<Renderable> Render();
    
private:	
    Liquid* liquid;
    std::vector<VelocityField*> currents;
    OpenGLOcean* glOcean;
    btScalar depth;
    GLfloat algeaBloom;
    GLfloat turbidity;
    bool trueWaves;
};
#endif
