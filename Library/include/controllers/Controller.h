//
//  Controller.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Controller__
#define __Stonefish_Controller__

#include "StonefishCommon.h"

namespace sf
{

typedef enum {CONTROLLER_FEEDBACK, CONTROLLER_PATHFOLLOWING, CONTROLLER_CUSTOM} ControllerType;

/*! Abstract class representing a general controller */
class Controller
{
public:
    Controller(std::string uniqueName, Scalar frequency);
    virtual ~Controller();
    
    void Start();
    void Stop();
    void Update(Scalar dt);
    virtual void Reset() = 0;
    
    Scalar getFrequency();
    Scalar getRunningTime();
    std::string getName();
    virtual ControllerType getType() = 0;
    
protected:
    virtual void Tick(Scalar dt) = 0;
    
    Scalar freq;
    Scalar runningTime;
    
private:
    std::string name;
    Scalar eleapsedTime;
    bool running;
};
    
}

#endif
