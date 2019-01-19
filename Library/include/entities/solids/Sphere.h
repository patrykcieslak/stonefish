//
//  Sphere.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sphere__
#define __Stonefish_Sphere__

#include "entities/SolidEntity.h"

namespace sf
{
    //! A class representing a sphere-shaped rigid body.
    class Sphere : public SolidEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sphere
         \param radius a radius of the sphere
         \param origin a transformation of the physical mesh (graphical is the same for sphere)
         \param m a material of the sphere
         \param lookId an index of the graphical material used to render the sphere
         \param thickness defines the thickness of the sphere walls, if positive the sphere is treated as shell
         \param enableHydrodynamicForces a flag to enable computation of hydrodynamic forces
         \param isBuoyant defines if the sphere should be subject to buoyancy force
         */
        Sphere(std::string uniqueName, Scalar radius, const Transform& origin, Material m, int lookId = -1, Scalar thickness = Scalar(-1),
               bool enableHydrodynamicForces = true, bool isBuoyant = true);
        
        //! A method that returns the type of body.
        SolidType getSolidType();
        
        //! A method that returns the collision shape for the sphere.
        btCollisionShape* BuildCollisionShape();
        
    private:
        Scalar r;
    };
}

#endif
