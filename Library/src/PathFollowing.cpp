//
//  PathFollowing.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "PathFollowing.h"

#pragma mark Constructors
PathFollowing::PathFollowing(std::string uniqueName, PathGenerator* pathGenerator, Trajectory* positionSensor, Controller* outputController, btScalar frequency) : Controller(uniqueName, frequency)
{
    inputPath = pathGenerator;
    measuredTraj = positionSensor;
    outputCtrl = outputController;
}

#pragma mark - Destructor
PathFollowing::~PathFollowing()
{
    delete inputPath;
    delete outputCtrl;
}

#pragma mark - Controller
void PathFollowing::Reset()
{
    
}

ControllerType PathFollowing::getType()
{
    return CONTROLLER_PATHFOLLOWING;
}

void PathFollowing::Tick(btScalar dt)
{
    //Calculate error
    
    
    //Run path following algorithm
    ControlTick(dt);
    
    //Update output
    outputCtrl->Update(dt);
}
