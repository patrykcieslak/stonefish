//
//  ResearchDynamicsWorld.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include <core/ResearchDynamicsWorld.h>
#include <BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h>
#include <BulletDynamics/Featherstone/btMultiBody.h>
#include <BulletDynamics/Featherstone/btMultiBodyLinkCollider.h>
#include "BulletCollision/CollisionDispatch/btSimulationIslandManager.h"
#include "LinearMath/btQuickprof.h"
#include <BulletDynamics/Featherstone/btMultiBodyConstraint.h>

ResearchDynamicsWorld::ResearchDynamicsWorld(btDispatcher* dispatcher, btBroadphaseInterface* pairCache, ResearchConstraintSolver* constraintSolver, btCollisionConfiguration* collisionConfiguration) : btMultiBodyDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration)
{
}