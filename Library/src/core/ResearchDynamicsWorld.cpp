//
//  ResearchDynamicsWorld.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "core/ResearchDynamicsWorld.h"

#include <BulletCollision/CollisionDispatch/btSimulationIslandManager.h>
#include <BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h>
#include <BulletDynamics/Featherstone/btMultiBody.h>
#include <BulletDynamics/Featherstone/btMultiBodyLinkCollider.h>
#include <BulletDynamics/Featherstone/btMultiBodyConstraint.h>
#include <LinearMath/btQuickprof.h>

using namespace sf;

ResearchDynamicsWorld::ResearchDynamicsWorld(btDispatcher* dispatcher, btBroadphaseInterface* pairCache, ResearchConstraintSolver* constraintSolver, btCollisionConfiguration* collisionConfiguration) : btMultiBodyDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration)
{
}
