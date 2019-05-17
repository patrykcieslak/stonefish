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
//  Created by Patryk Cieslak on 3/5/14.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
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
         \param bpt an enum defining the type of physics computations required for the body (currently bodies cannot transfer between mediums)
         \param lookId an index of the graphical material used to render the torus
         \param thickness defines the thickness of the torus walls, if positive the torus is treated as shell
         \param isBuoyant defines if the torus should be subject to buoyancy force
         */
        Torus(std::string uniqueName, Scalar majorRadius, Scalar minorRadius, const Transform& origin,
              Material m, BodyPhysicsType bpt, int lookId = -1, Scalar thickness = Scalar(-1), bool isBuoyant = true);
        
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
