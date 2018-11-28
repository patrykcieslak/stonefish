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

namespace sf
{

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
    Ocean(std::string uniqueName, bool simulateWaves, Fluid* l);
    virtual ~Ocean();
    
    //Forces
    void AddVelocityField(VelocityField* field);
    virtual void ApplyFluidForces(const HydrodynamicsType ht, btDynamicsWorld* world, btCollisionObject* co, bool recompute);
    virtual Vector3 GetFluidVelocity(const Vector3& point) const;
    void GetSurface(Vector3& normal, Vector3& position) const;
    void GetSurfaceEquation(double* plane4) const;
    bool IsInsideFluid(const Vector3& point);
    Scalar GetPressure(const Vector3& point);
    Scalar GetDepth(const Vector3& point);
    glm::vec3 ComputeLightAbsorption();
    
    //Getters
    bool hasWaves();
    void setAlgeaBloomFactor(GLfloat f);
    void setTurbidity(GLfloat ntu);
    GLfloat getAlgeaBloomFactor();
    GLfloat getTurbidity();
    const Fluid* getLiquid() const;
    OpenGLOcean* getOpenGLOcean();
    ForcefieldType getForcefieldType();
    
	void InitGraphics(SDL_mutex* hydrodynamics);
    std::vector<Renderable> Render();
    
private:	
    Fluid* liquid;
    std::vector<VelocityField*> currents;
    OpenGLOcean* glOcean;
    Scalar depth;
    GLfloat algeaBloom;
    GLfloat turbidity;
    bool waves;
    Renderable wavesDebug;
};
    
}

#endif
