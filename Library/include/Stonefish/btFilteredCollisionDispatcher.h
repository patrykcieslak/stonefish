//
//  btFilteredCollisionDispatcher.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_btFilteredCollisionDispatcher__
#define __Stonefish_btFilteredCollisionDispatcher__

#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>

class btFilteredCollisionDispatcher : public btCollisionDispatcher
{
public:
    btFilteredCollisionDispatcher(btCollisionConfiguration* collisionConfiguration);
    
    bool needsCollision(const btCollisionObject* body0, const btCollisionObject* body1);
};

#endif
