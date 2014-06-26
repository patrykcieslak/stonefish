//
//  ResearchDynamicsWorld.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "ResearchDynamicsWorld.h"
#include "btMultiBodyConstraintSolver.h"
#include "btMultiBody.h"
#include "btMultiBodyLinkCollider.h"
#include "BulletCollision/CollisionDispatch/btSimulationIslandManager.h"
#include "LinearMath/btQuickprof.h"
#include "btMultiBodyConstraint.h"

ResearchDynamicsWorld::ResearchDynamicsWorld(btDispatcher* dispatcher, btBroadphaseInterface* pairCache, ResearchConstraintSolver* constraintSolver, btCollisionConfiguration* collisionConfiguration) : btMultiBodyDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration)
{
}