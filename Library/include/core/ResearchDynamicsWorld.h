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
//  ResearchDynamicsWorld.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ResearchDynamicsWorld__
#define __Stonefish_ResearchDynamicsWorld__

#include <BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h>

namespace sf
{
    class ResearchConstraintSolver;
    
    //! A class representing the physics world.
    class ResearchDynamicsWorld : public btMultiBodyDynamicsWorld
    {
    public:
        //! A constructor.
        /*!
         \param dispatcher a pointer to the collision dispatcher
         \param pairCache a pointer to the collision cache
         \param constraintSover a pointer to the constraint solver
         \param collisionConfiguration a pointer to the collision configuration structure
         */
        ResearchDynamicsWorld(btDispatcher* dispatcher,
                              btBroadphaseInterface* pairCache,
                              ResearchConstraintSolver* constraintSolver,
                              btCollisionConfiguration* collisionConfiguration);
    };
}

#endif
