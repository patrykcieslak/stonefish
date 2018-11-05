//
//  Controller.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Controller__
#define __Stonefish_Controller__

#include <common.h>

typedef enum {CONTROLLER_FEEDBACK, CONTROLLER_PATHFOLLOWING, CONTROLLER_CUSTOM} ControllerType;

/*! Abstract class representing a general controller */
class Controller
{
public:
    Controller(std::string uniqueName, btScalar frequency = btScalar(-1.));
    virtual ~Controller();
    
    void Start();
    void Stop();
    void Update(btScalar dt);
    virtual void Reset() = 0;
    
    btScalar getFrequency();
    btScalar getRunningTime();
    std::string getName();
    virtual ControllerType getType() = 0;
    
protected:
    virtual void Tick(btScalar dt) = 0;
    
    btScalar freq;
    btScalar runningTime;
    
private:
    std::string name;
    btScalar eleapsedTime;
    bool running;
};

#endif