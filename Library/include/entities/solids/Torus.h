//
//  Torus.h
//  Stonefish
//
//  Created by Patryk Cieslak on 3/5/14.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Torus__
#define __Stonefish_Torus__

#include "entities/SolidEntity.h"

namespace sf
{
    //! A class representing a torus-shaped rigid body.
    class Torus : public SolidEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the torus
         \param majorRadius a major radius of the torus
         \param minorRadius a minor radius of the torus
         \param origin a transformation of the physical mesh (graphical is the same for torus)
         \param m a material of the torus
         \param lookId an index of the graphical material used to render the torus
         \param thickness defines the thickness of the torus walls, if positive the torus is treated as shell
         \param enableHydrodynamicForces a flag to enable computation of hydrodynamic forces
         \param isBuoyant defines if the torus should be subject to buoyancy force
         */
        Torus(std::string uniqueName, Scalar majorRadius, Scalar minorRadius, const Transform& origin,
              Material m, int lookId = -1, Scalar thickness = Scalar(-1), bool enableHydrodynamicForces = true, bool isBuoyant = true);
        
        //! A method that returns the type of body.
        SolidType getSolidType();
        
        //! A method that returns the collision shape for the torus.
        btCollisionShape* BuildCollisionShape();
        
    private:
        Scalar mR;
        Scalar MR;
    };
}

#endif
