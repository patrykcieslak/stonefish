//
//  Ocean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Ocean__
#define __Stonefish_Ocean__

#include <SDL2/SDL_mutex.h>
#include "core/MaterialManager.h"
#include "entities/ForcefieldEntity.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    typedef enum {TRIFOLD_SYMMETRY, FULLY_COUPLED, GEOMETRY_BASED} HydrodynamicsType;
    
    struct HydrodynamicsSettings
    {
        HydrodynamicsType algorithm;
        bool dampingForces;
        bool reallisticBuoyancy;
    };
    
    class VelocityField;
    class OpenGLOcean;
    
    class Ocean : public ForcefieldEntity
    {
    public:
        Ocean(std::string uniqueName, bool simulateWaves, Fluid* l);
        ~Ocean();
        
        void SetupWaterProperties(Scalar type, Scalar turbidity);
        
        //Forces
        void AddVelocityField(VelocityField* field);
        virtual void ApplyFluidForces(const HydrodynamicsType ht, btDynamicsWorld* world, btCollisionObject* co, bool recompute);
        virtual Vector3 GetFluidVelocity(const Vector3& point) const;
        void GetSurface(Vector3& normal, Vector3& position) const;
        void GetSurfaceEquation(double* plane4) const;
        bool IsInsideFluid(const Vector3& point);
        Scalar GetPressure(const Vector3& point);
        Scalar GetDepth(const Vector3& point);
        
        //Accessors
        Scalar getWaterType();
        Scalar getTurbidity();
        bool hasWaves();
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
        Scalar waterType;
        Scalar turbidity;
        bool waves;
        Renderable wavesDebug;
    };
}

#endif
