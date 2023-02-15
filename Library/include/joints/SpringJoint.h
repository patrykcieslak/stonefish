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
//  SpringJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/02/23.
//  Copyright (c) 2023 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SpringJoint__
#define __Stonefish_SpringJoint__

#include "joints/Joint.h"

namespace sf
{
    class SolidEntity;
    
    //! A class representing a spring.
    class SpringJoint : public Joint
    {
    public:
        //! A constructor to create a spring between solid and world.
        /*!
         \param uniqueName a name for the joint
         \param solid a pointer to the solid body
         \param attachment the frame to which the spring is attached
         \param linearStiffness stiffness of the spring for the linear DOFs
         \param angularStiffness stiffness of the spring for the angular DOFs
         \param linearDamping damping for the linear DOFs
         \param angularDamping damping for the angular DOFs
         */
        SpringJoint(std::string uniqueName, SolidEntity* solid, const Transform& attachment, 
            const Vector3& linearStiffness, const Vector3& angularStiffness,
            const Vector3& linearDamping, const Vector3& angularDamping);

        //! A constructor to create fixed joint between two solid bodies.
        /*!
         \param uniqueName a name for the joint
         \param solidA a pointer to the first solid body
         \param solidB a pointer to the second solid body
         \param attachment the frame to which the spring is attached
         \param linearStiffness stiffness of the spring for the linear DOFs
         \param angularStiffness stiffness of the spring for the angular DOFs
         \param linearDamping damping for the linear DOFs
         \param angularDamping damping for the angular DOFs
         \
         */
        SpringJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Transform& attachment,
            const Vector3& linearStiffness, const Vector3& angularStiffness,
            const Vector3& linearDamping, const Vector3& angularDamping);
        
        //! A method implementing the rendering of the joint.
        std::vector<Renderable> Render();
        
        //! A method returning the type of the joint.
        JointType getType() const;
    };
}
    
#endif
