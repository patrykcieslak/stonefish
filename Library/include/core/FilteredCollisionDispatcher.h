//
//  FilteredCollisionDispatcher.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FilteredCollisionDispatcher__
#define __Stonefish_FilteredCollisionDispatcher__

#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>

namespace sf
{
    //! A class implementing a custom collision dispatcher object.
    class FilteredCollisionDispatcher : public btCollisionDispatcher
    {
    public:
        //! A constructor.
        FilteredCollisionDispatcher(btCollisionConfiguration* collisionConfiguration, bool inclusiveMode);
        
        //! A method that informs if two collision objects can collide.
        bool needsCollision(const btCollisionObject* body0, const btCollisionObject* body1);
        
        //! A method calling the collision computation algorithm.
        static void myNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);
        
    private:
        bool inclusive;
    };
}

#endif
