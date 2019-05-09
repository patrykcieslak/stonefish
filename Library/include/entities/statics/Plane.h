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
//  Plane.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Plane__
#define __Stonefish_Plane__

#include "entities/StaticEntity.h"

namespace sf
{
    //! A class representing an infinite static plane.
    class Plane : public StaticEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the plane
         \param size of the rendered plane [m]
         \param m the material of the plane
         \param lookId the id of the material used to render the plane
         */
        Plane(std::string uniqueName, Scalar size, Material m, int lookId = -1);
        
        //! A method returning the extents of the plane axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        void getAABB(Vector3& min, Vector3& max);
        
        //! A method returning the type of the static entity.
        StaticEntityType getStaticType();
    };
}

#endif
