//
//  AcroverPathFollowingController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "AcroverPathFollowingController.h"

#pragma mark - Constructors
AcroverPathFollowingController::AcroverPathFollowingController(std::string uniqueName, PathGenerator2D* pathGenerator, Trajectory* robotPosition, RotaryEncoder* wheelEncoder, AcroverSpeedController* speedController, AcroverTiltController* tiltController, btScalar frequency) : PathFollowingController(uniqueName, pathGenerator, robotPosition, frequency)
{
    outputControllers.push_back(speedController);
    outputControllers.push_back(tiltController);
    velocity = btScalar(0.3);
    positionErrorIntegral = btScalar(0.);
    distanceErrorIntegral = btScalar(0.);
    wheelEnc = wheelEncoder;
}

#pragma mark - Methods
btScalar AcroverPathFollowingController::VelocityOnPath()
{
    return velocity;
}

void AcroverPathFollowingController::ControlTick(btScalar dt)
{
    //Orientation
    btScalar omega = -wheelEnc->getLastSample().getValue(1);
    std::vector<btScalar> measured = measuredTraj->getLastSample().getData();
    
    btScalar actualOrientation = measured[5]; //Yaw
    btVector3 velocity(sin(actualOrientation), cos(actualOrientation), 0.0);
    btVector3 errorV(error[0], error[1], 0.0);
    btScalar dir = velocity.dot(errorV);
    //printf("Dot: %1.5f\n", dir);
    
    //Forward control signal (PI)
    btScalar positionError = btSqrt(error[0] * error[0] + error[1] * error[1]);
    if(omega != 0.0)
        positionError = dir*omega > 0 ? -positionError : positionError;
    else
        positionError = dir > 0 ? positionError : -positionError;
    
    btScalar speedControl = 5.0 * positionError + 0.0 * positionErrorIntegral;
    positionErrorIntegral += positionError * dt;
    
    //Tilt control signal (P + PI)
    
    btScalar tiltControl = btAtan2(-(30.0 * error[3] + 20.0 * error[2] + 5.0 * distanceErrorIntegral) * omega * 0.240, 9.81);
    distanceErrorIntegral += error[2] * dt;
    
    //printf("Speed: %1.5f  Tilt: %1.5f\n", -speedControl, tiltControl);
    
    AcroverSpeedController* longitudinal = (AcroverSpeedController*)outputControllers[0];
    longitudinal->setDesiredSpeed(speedControl);
    
    AcroverTiltController* lateral = (AcroverTiltController*)outputControllers[1];
    lateral->setDesiredTilt(tiltControl);
}

void AcroverPathFollowingController::PathEnd()
{
    AcroverSpeedController* longitudinal = (AcroverSpeedController*)outputControllers[0];
    longitudinal->setDesiredSpeed(0);
    
    AcroverTiltController* lateral = (AcroverTiltController*)outputControllers[1];
    lateral->setDesiredTilt(0);
}