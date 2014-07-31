//
//  MonoWheelPathFollowing.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "MonoWheelPathFollowing.h"

#pragma mark - Constructors
MonoWheelPathFollowing::MonoWheelPathFollowing(std::string uniqueName, PathGenerator2D* pathGenerator, Trajectory* robotPosition, RotaryEncoder* wheelEncoder, MonoWheelLateral* lateralController, MISOStateSpaceController* longitudinalController, btScalar frequency) : PathFollowingController(uniqueName, pathGenerator, robotPosition, frequency)
{
    outputControllers.push_back(lateralController);
    outputControllers.push_back(longitudinalController);
    velocity = btScalar(0.25);
    positionErrorIntegral = btScalar(0.);
    distanceErrorIntegral = btScalar(0.);
    wheelEnc = wheelEncoder;
}

#pragma mark - Methods
btScalar MonoWheelPathFollowing::VelocityOnPath()
{
    return velocity;
}

void MonoWheelPathFollowing::ControlTick(btScalar dt)
{
    //Forward control signal (PI)
    btScalar positionError = btSqrt(error[0] * error[0] + error[1] * error[1]) - 0.2;
    positionErrorIntegral += positionError * dt;
    
    btScalar longitudinalControl = 2.0 * positionError + 1.0 * positionErrorIntegral;
    
    MISOStateSpaceController* longitudinal = (MISOStateSpaceController*)outputControllers[1];
    longitudinal->setReferenceValue(2, -longitudinalControl);
    
    //Tilt control signal (P + PI)
    btScalar omega = wheelEnc->getLastSample().getValue(1);
    distanceErrorIntegral += error[2] * dt;
    btScalar lateralControl = btAtan2((20.0 * error[3] + 100.0 * error[2] + 10.0 * distanceErrorIntegral) * omega * 0.205, 9.81);
    
    MonoWheelLateral* lateral = (MonoWheelLateral*)outputControllers[0];
    lateral->setReferenceValue(0, lateralControl);
}

void MonoWheelPathFollowing::PathEnd()
{
    MISOStateSpaceController* longitudinal = (MISOStateSpaceController*)outputControllers[1];
    longitudinal->setReferenceValue(2, 0.0);
    
    MonoWheelLateral* lateral = (MonoWheelLateral*)outputControllers[0];
    lateral->setReferenceValue(0, 0.0);
}