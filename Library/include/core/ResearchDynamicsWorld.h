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
