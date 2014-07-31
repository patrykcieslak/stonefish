//
//  MonoWheelPathFollowing.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MonoWheelPathFollowing__
#define __Stonefish_MonoWheelPathFollowing__

#include "PathFollowingController.h"
#include "PathGenerator2D.h"
#include "MISOStateSpaceController.h"
#include "MonoWheelLateral.h"
#include "RotaryEncoder.h"

class MonoWheelPathFollowing : public PathFollowingController
{
public:
    MonoWheelPathFollowing(std::string uniqueName, PathGenerator2D* pathGenerator, Trajectory* robotPosition, RotaryEncoder* wheelEncoder, MonoWheelLateral* lateralController, MISOStateSpaceController* longitudinalController, btScalar frequency = btScalar(1.));
    
private:
    btScalar VelocityOnPath();
    void ControlTick(btScalar dt);
    void PathEnd();
    
    btScalar velocity;
    btScalar positionErrorIntegral;
    btScalar distanceErrorIntegral;
    RotaryEncoder* wheelEnc;
};

#endif
