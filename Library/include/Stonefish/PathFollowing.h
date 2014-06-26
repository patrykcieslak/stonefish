//
//  PathFollowing.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PathFollowing__
#define __Stonefish_PathFollowing__

#include "Controller.h"
#include "PathGenerator.h"
#include "Trajectory.h"

/*! Abstract path following controller base class */
class PathFollowing : public Controller
{
public:
    PathFollowing(std::string uniqueName, PathGenerator* pathGenerator, Trajectory* positionSensor, Controller* outputController, btScalar frequency = btScalar(1.));
    virtual ~PathFollowing();
    
    virtual void Reset();
    ControllerType getType();
    
protected:
    void Tick(btScalar dt); //For updating path generator, calculating error and updating controller
    virtual void ControlTick(btScalar dt) = 0; //For path following control algorithm
    
    PathGenerator* inputPath;
    Trajectory* measuredTraj;
    Controller* outputCtrl;
};

#endif
