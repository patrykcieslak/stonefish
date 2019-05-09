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
//  Box.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Box__
#define __Stonefish_Box__

#include "entities/SolidEntity.h"

namespace sf
{
    //! A class representing a box-shaped rigid body.
    class Box : public SolidEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the box
         \param dimensions a vector of box dimensions (side lengths)
         \param origin a transformation of the physical mesh (graphical is the same for box)
         \param m a material of the box
         \param bpt an enum defining the type of physics computations required for the body (currently bodies cannot transfer between mediums)
         \param lookId an index of the graphical material used to render the box
         \param thickness defines the thickness of the box walls, if positive the box is treated as shell
         \param isBuoyant defines if the box should be subject to buoyancy force
        */
        Box(std::string uniqueName, const Vector3& dimensions, const Transform& origin, Material m, BodyPhysicsType bpt,
                                    int lookId = -1, Scalar thickness = Scalar(-1), bool isBuoyant = true);
        
        //! A method that returns the type of body.
        SolidType getSolidType();
        
        //! A method that returns the collision shape for the box.
        btCollisionShape* BuildCollisionShape();
        
    private:
        Vector3 halfExtents;
    };
}

#endif
