//
//  PathFollowingController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PathFollowingController__
#define __Stonefish_PathFollowingController__

#include "controllers/Controller.h"

namespace sf
{
    class PathGenerator;
    class Trajectory;
    
    //! Abstract path following controller base class.
    class PathFollowingController : public Controller
    {
    public:
        PathFollowingController(std::string uniqueName, PathGenerator* pathGenerator, Trajectory* positionSensor, Scalar frequency);
        virtual ~PathFollowingController();
        
        void RenderPath();
        virtual void Reset();
        PathGenerator* getPath();
        ControllerType getType();
        
    protected:
        void Tick(Scalar dt); //updating path generator, calculating error and updating controller
        virtual Scalar VelocityOnPath() = 0; //calculate velocity to update position on path
        virtual void ControlTick(Scalar dt) = 0; //path following control algorithm
        virtual void PathEnd() = 0; //path end control algorithm
        
        PathGenerator* inputPath;
        Trajectory* measuredTraj;
        std::vector<Controller*> outputControllers;
        std::vector<Scalar> error;
    };
}
    
#endif
