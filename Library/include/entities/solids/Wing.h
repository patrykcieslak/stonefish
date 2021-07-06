/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  Wing.h
//  Stonefish
//
//  Created by Patryk Cieślak on 17/01/2019.
//  Copyright (c) 2019-2021 Patryk Cieslak. All rights reserved.
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
         \param phy the specific settings of the physics computation for the body
         \param baseChordLength the length of the chord at the base of the wing [m]
         \param tipChordLength the length of the chord at the tip of the wing [m]
         \param maxCamber the maximum camber as percent of the chord length
         \param maxCamberPos the position of the maximum camber point as percent of chord length
         \param profileThickness the thickness of the profile as percent of chord length
         \param wingLength the length of the wing [m]
         \param origin a transformation of the physical mesh (graphical is the same for wing)
         \param material the name of the material the wing is made of
         \param look the name of the graphical material used for rendering
         \param thickness defines the thickness of the wing walls, if positive the wing is treated as shell [m]
         */
        Wing(std::string uniqueName, BodyPhysicsSettings phy, Scalar baseChordLength, Scalar tipChordLength,
             Scalar maxCamber, Scalar maxCamberPos, Scalar profileThickness, Scalar wingLength, const Transform& origin, 
             std::string material, std::string look, Scalar thickness = Scalar(-1));
        
        //! A constructor.
        /*!
         \param uniqueName a name for the wing
         \param phy the specific settings of the physics computation for the body
         \param baseChordLength the length of the chord at the base of the wing [m]
         \param tipChordLength the length of the chord at the tip of the wing [m]
         \param NACA the code of the profile in 4-digit NACA system
         \param wingLength the length of the wing [m]
         \param origin a transformation of the physical mesh (graphical is the same for wing)
         \param material the name of the material the wing is made of
         \param look the name of the graphical material used for rendering
         \param thickness defines the thickness of the wing walls, if positive the wing is treated as shell
         */
        Wing(std::string uniqueName, BodyPhysicsSettings phy, Scalar baseChordLength, Scalar tipChordLength, std::string NACA, Scalar wingLength, const Transform& origin, 
             std::string material, std::string look, Scalar thickness = Scalar(-1));
        
        //! A method that returns the type of body.
        SolidType getSolidType();
        
        //! A method that returns the collision shape for the wing (tapered box).
        btCollisionShape* BuildCollisionShape();
        
    private:
        
    };
}

#endif
