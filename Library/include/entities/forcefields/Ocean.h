//
//  Ocean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright (c) 2017-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Ocean__
#define __Stonefish_Ocean__

#include <SDL2/SDL_mutex.h>
#include "core/MaterialManager.h"
#include "entities/ForcefieldEntity.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    //! An enum specifying the type of hydrodynamics computation that is used in the simulation
    typedef enum {TRIFOLD_SYMMETRY, FULLY_COUPLED, GEOMETRY_BASED} HydrodynamicsType;
    
    //! A structure holding the settings of the hydrodynamics computation.
    struct HydrodynamicsSettings
    {
        HydrodynamicsType algorithm;
        bool dampingForces;
        bool reallisticBuoyancy;
    };
    
    class VelocityField;
    class OpenGLOcean;
    
    //! A class implementing an ocean.
    class Ocean : public ForcefieldEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the ocean
         \param simulateWaves a flag to decide if waves should be simulated
         \param l a pointer to the fluid that is filling the ocean (normally water)
         */
        Ocean(std::string uniqueName, bool simulateWaves, Fluid* l);
        
        //! A destructor.
        ~Ocean();
        
        //! A method used to setup the properties of the water.
        /*!
         \param type the type of the water <0,1>
         \param turbidity the turbidity of the water
         */
        void SetupWaterProperties(Scalar type, Scalar turbidity);
        
        //! A method used to add a velocity field to the ocean.
        /*!
         \param field a pointer to a velocity field object
         */
        void AddVelocityField(VelocityField* field);
        
        //! A method running the hydrodynamics computation.
        /*!
         \param ht type of hydrodynamics computations
         \param world a pointer to the dynamics world
         \param co a pointer to the collision object
         \param recompute a flag deciding if hydrodynamic forces need to be recomputed
         */
        virtual void ApplyFluidForces(const HydrodynamicsType ht, btDynamicsWorld* world, btCollisionObject* co, bool recompute);
        
        //! A method returning the water velocity.
        /*!
         \param point the point in the ocean where the velocity should be measured [m]
         \return fluid velocity at specified point [m/s]
         */
        virtual Vector3 GetFluidVelocity(const Vector3& point) const;
        
        //! A method checking if a point is inside fluid
        /*!
         \param point the position of a point to be checked [m]
         \return is the point inside fluid?
         */
        bool IsInsideFluid(const Vector3& point);
        
        //! A method returning the hydrostatic pressure of the fluid at the specified point.
        /*!
         \param point the position of the measurement point [m]
         \return the hydrostatic pressure [Pa]
         */
        Scalar GetPressure(const Vector3& point);
        
        //! A method returning the depth of the ocean at the specified point.
        /*!
         \param point the position of the measurement point [m]
         \return the distance from the point to the surfacer of fluid [m]
         */
        Scalar GetDepth(const Vector3& point);
        
        //! A method returning the type of the water.
        Scalar getWaterType();
        
        //! A method returning the turbidity of the water.
        Scalar getTurbidity();
        
        //! A method informing if the ocean waves are simulated.
        bool hasWaves();
        
        //! A method returning a pointer to the fluid filling the ocean.
        const Fluid* getLiquid() const;
        
        //! A method returning a pointer to the OpenGL object implementing the ocean.
        OpenGLOcean* getOpenGLOcean();
        
        //! A method returning the type of the force field.
        ForcefieldType getForcefieldType();
        
        //! A method initializing the rendering of the ocean.
        /*!
         \param hydrodynamics a pointer to a mutex
         */
        void InitGraphics(SDL_mutex* hydrodynamics);
        
        //! A method implementing the rendering of the ocean force field.
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
