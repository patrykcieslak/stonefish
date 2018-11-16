//
//  ResearchDynamicsWorld.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ResearchDynamicsWorld__
#define __Stonefish_ResearchDynamicsWorld__

#include <BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h>
#include "core/ResearchConstraintSolver.h"

class ResearchDynamicsWorld : public btMultiBodyDynamicsWorld
{
public:
    ResearchDynamicsWorld(btDispatcher* dispatcher, btBroadphaseInterface* pairCache, ResearchConstraintSolver* constraintSolver, btCollisionConfiguration* collisionConfiguration);
};

#endif
