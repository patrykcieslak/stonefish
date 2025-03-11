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
//  Trigger.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/04/18.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Trigger__
#define __Stonefish_Trigger__

#include "entities/ForcefieldEntity.h"
#include "entities/SolidEntity.h"

namespace sf
{
    //! A class implementing a virtual object that can be used to trigger actions.
    class Trigger : public ForcefieldEntity
    {
    public:
        //! A constructor of a spherical trigger object.
        /*!
         \param uniqueName a name for the trigger
         \param radius the radius of the sphere [m]
         \param origin the position of the trigger in the world frame
         \param look a name of the material used when rendering the trigger
         */
        Trigger(std::string uniqueName, Scalar radius, const Transform& origin, std::string look = "");
        
        //! A constructor of a cylindrical trigger object.
        /*!
         \param uniqueName a name for the trigger
         \param radius the radius of the cylinder [m]
         \param length the length of the cylinder [m]
         \param origin the position of the trigger in the world frame
         \param look a name of the material used when rendering the trigger
         */
        Trigger(std::string uniqueName, Scalar radius, Scalar length, const Transform& origin, std::string look = "");
        
        //! A constructor of a box-shaped trigger object.
        /*!
         \param uniqueName a name for the trigger
         \param dimensions the length of the box sides [m]
         \param origin the position of the trigger in the world frame
         \param look a name of the material used when rendering the trigger
         */
        Trigger(std::string uniqueName, const Vector3& dimensions, const Transform& origin, std::string look = "");
        
        //! A method used to add solids that will call actions if they come in contact with the trigger.
        /*!
         \param solid a pointer to a rigid body
         */
        void AddActiveSolid(SolidEntity* solid);
        
        //! A method used to activate actions for a collision object.
        /*!
         \param co a pointer to a collision object
         */
        void Activate(btCollisionObject* co);
        
        //! A method that clears the activity flag.
        void Clear();
        
        //! A method implementing the rendering of the trigger.
        std::vector<Renderable> Render();
        
        //! A method returning the activity status.
        bool isActive();
        
        //! A method returning the force field type.
        ForcefieldType getForcefieldType();
        
    private:
        bool active;
        std::vector<SolidEntity*> solids;
        int objectId;
        int lookId;
    };
}

#endif
