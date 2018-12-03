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
        ResearchDynamicsWorld(btDispatcher* dispatcher,
                              btBroadphaseInterface* pairCache,
                              ResearchConstraintSolver* constraintSolver,
                              btCollisionConfiguration* collisionConfiguration);
    };
}

#endif
