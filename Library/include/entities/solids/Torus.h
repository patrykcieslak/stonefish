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
//  Torus.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/01/13.
//  Copyright (c) 2013-2021 Patryk Cieslak. All rights reserved.
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
         \param phy the specific settings of the physics computation for the body
         \param majorRadius a major radius of the torus
         \param minorRadius a minor radius of the torus
         \param origin a transformation of the physical mesh (graphical is the same for torus)
         \param material the name of the material the torus is made of
         \param look the name of the graphical material used for rendering
         \param thickness defines the thickness of the torus walls, if positive the torus is treated as shell
         */
        Torus(std::string uniqueName, BodyPhysicsSettings phy, Scalar majorRadius, Scalar minorRadius, const Transform& origin,
              std::string material, std::string look, Scalar thickness = Scalar(-1));
        
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
