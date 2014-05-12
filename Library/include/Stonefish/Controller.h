//
//  Controller.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Controller__
#define __Stonefish_Controller__

#include "common.h"
#include "NameManager.h"

typedef enum {SERVO, CUSTOM} ControllerType;

//abstract class
class Controller
{
public:
    Controller(std::string uniqueName, btScalar frequency);
    virtual ~Controller();
    
    void Start();
    void Stop();
    void Update(btScalar dt);
    
    virtual void Reset() = 0;
    virtual ControllerType getType() = 0;
    
    std::string getName();
    btScalar getFrequency();
    
protected:
    virtual void Tick() = 0;

    btScalar freq;

private:
    std::string name;
    btScalar eleapsedTime;
    bool running;
    
    static NameManager nameManager;
};

#endif