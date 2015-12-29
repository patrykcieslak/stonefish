//
//  AcroverPathFollowingController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_AcroverPathFollowingController__
#define __Stonefish_AcroverPathFollowingController__

#include "PathFollowingController.h"
#include "PathGenerator2D.h"
#include "AcroverSpeedController.h"
#include "AcroverTiltController.h"
#include "RotaryEncoder.h"

class AcroverPathFollowingController : public PathFollowingController
{
public:
    AcroverPathFollowingController(std::string uniqueName, PathGenerator2D* pathGenerator, Trajectory* robotPosition, RotaryEncoder* wheelEncoder, AcroverSpeedController* speedController, AcroverTiltController* tiltController, btScalar frequency = btScalar(1.));
    
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
