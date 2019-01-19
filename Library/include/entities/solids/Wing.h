//
//  Wing.h
//  Stonefish
//
//  Created by Patryk Cieślak on 17/01/2019.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Wing__
#define __Stonefish_Wing__

#include "entities/SolidEntity.h"

namespace sf
{
    //! A class representing a wing-shaped rigid body. The profile of the wing is generated based on 4-digit NACA system.
    class Wing : public SolidEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the wing
         \param baseChordLength the length of the chord at the base of the wing [m]
         \param tipChordLength the length of the chord at the tip of the wing [m]
         \param maxCamber the maximum camber as percent of the chord length
         \param maxCamberPos the position of the maximum camber point as percent of chord length
         \param profileThickness the thickness of the profile as percent of chord length
         \param wingLength the length of the wing [m]
         \param origin a transformation of the physical mesh (graphical is the same for wing)
         \param m a material of the wing
         \param lookId an index of the graphical material used to render the wing
         \param thickness defines the thickness of the wing walls, if positive the wing is treated as shell [m]
         \param enableHydrodynamicForces a flag to enable computation of hydrodynamic forces
         \param isBuoyant defines if the wing should be subject to buoyancy force
         */
        Wing(std::string uniqueName, Scalar baseChordLength, Scalar tipChordLength,
             Scalar maxCamber, Scalar maxCamberPos, Scalar profileThickness, Scalar wingLength, const Transform& origin, Material m,
            int lookId = -1, Scalar thickness = Scalar(-1), bool enableHydrodynamicForces = true, bool isBuoyant = true);
        
        //! A constructor.
        /*!
         \param uniqueName a name for the wing
         \param baseChordLength the length of the chord at the base of the wing [m]
         \param tipChordLength the length of the chord at the tip of the wing [m]
         \param NACA the code of the profile in 4-digit NACA system
         \param wingLength the length of the wing [m]
         \param origin a transformation of the physical mesh (graphical is the same for wing)
         \param m a material of the wing
         \param lookId an index of the graphical material used to render the wing
         \param thickness defines the thickness of the wing walls, if positive the wing is treated as shell
         \param enableHydrodynamicForces a flag to enable computation of hydrodynamic forces
         \param isBuoyant defines if the wing should be subject to buoyancy force
         */
        Wing(std::string uniqueName, Scalar baseChordLength, Scalar tipChordLength, std::string NACA, Scalar wingLength,
             const Transform& origin, Material m, int lookId = -1, Scalar thickness = Scalar(-1), bool enableHydrodynamicForces = true, bool isBuoyant = true);
        
        //! A method that returns the type of body.
        SolidType getSolidType();
        
        //! A method that returns the collision shape for the wing (tapered box).
        btCollisionShape* BuildCollisionShape();
        
    private:
        
    };
}

#endif
