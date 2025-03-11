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
//  FilteredCollisionDispatcher.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FilteredCollisionDispatcher__
#define __Stonefish_FilteredCollisionDispatcher__

#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"

namespace sf
{
    //! A class implementing a custom collision dispatcher object.
    class FilteredCollisionDispatcher : public btCollisionDispatcher
    {
    public:
        //! A constructor.
        /*!
         \param collisionConfiguration a pointer to the collision configuration structure
         \param inclusiveMode a flag that selects the mode of collision detection
         */
        FilteredCollisionDispatcher(btCollisionConfiguration* collisionConfiguration, bool inclusiveMode);
        
        //! A method that informs if two collision objects can collide.
        /*!
         \param body0 a pointer to the first collision object
         \param body1 a pointer to the second collision object
         \return can given objects collide?
         */
        bool needsCollision(const btCollisionObject* body0, const btCollisionObject* body1);
        
        //! A method calling the collision computation algorithm.
        /*!
         \param collisionPair a reference to a collision pair info structure
         \param dispatcher a reference to the collision dispatcher
         \param dispatchInfo a reference to the collision dispatcher info structure
         */
        static void myNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);
        
    private:
        bool inclusive;
    };
}

#endif
