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
//  FixedJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013-2023 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FixedJoint__
#define __Stonefish_FixedJoint__

#include "joints/Joint.h"

namespace sf
{
    class SolidEntity;
    class FeatherstoneEntity;
    
    //! A class representing a fixed joint.
    class FixedJoint : public Joint
    {
    public:
        //! A constructor to create fixed joint between solid and world.
        /*!
         \param uniqueName a name for the joint
         \param solid a pointer to the solid body
         */
        FixedJoint(std::string uniqueName, SolidEntity* solid);

        //! A constructor to create fixed joint between two solid bodies.
        /*!
         \param uniqueName a name for the joint
         \param solidA a pointer to the first solid body
         \param solidB a pointer to the second solid body
         */
        FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB);
        
        //! A constructor to create fixed joint between a solid body and a link of a multibody.
        /*!
         \param uniqueName a name for the joint
         \param solid a pointer to the solid body
         \param fe a pointer to the multibody
         \param linkId an index of the link of the multibody
         \param pivot a connection point
         */
        FixedJoint(std::string uniqueName, SolidEntity* solid, FeatherstoneEntity* fe, int linkId, const Vector3& pivot);
        
        //! A constructor to create fixed joint between two multibodies.
        /*!
         \param uniqueName a name for the joint
         \param feA a pointer to the first multibody
         \param feB a pointer to the second multibody
         \param linkIdA an index of the link of the first multibody
         \param linkIdB an index of the link of the second multibody
         \param pivot a connection point
         */
        FixedJoint(std::string uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB, const Vector3& pivot);

        //! A method implementing the rendering of the joint.
        std::vector<Renderable> Render();
        
        //! A method returning the type of the joint.
        JointType getType() const;
    };
}
    
#endif
