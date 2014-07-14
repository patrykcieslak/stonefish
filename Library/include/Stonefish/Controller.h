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
#include "SignalMux.h"

typedef enum {CONTROLLER_DCSERVO, CONTROLLER_MISO, CONTROLLER_PATHFOLLOWING, CONTROLLER_CUSTOM} ControllerType;

/*! Abstract base controller class */
class Controller
{
public:
    Controller(std::string uniqueName, btScalar frequency = btScalar(-1.));
    virtual ~Controller();
    
    void Start();
    void Stop();
    void Update(btScalar dt);
    virtual void Reset() = 0;
    
    void setReferenceSignalGenerator(unsigned int inputId, SignalGenerator* sg);
    void setReferenceSignalMux(SignalMux* sm);
    virtual ControllerType getType() = 0;
    virtual unsigned int getNumOfInputs() = 0;
    btScalar getFrequency();
    btScalar getRunningTime();
    std::string getName();
    
protected:
    virtual void Tick(btScalar dt) = 0;
    
    btScalar freq;
    SignalGenerator* referenceGen;
    unsigned int referenceInput;
    SignalMux* referenceMux;
    btScalar runningTime;
    
private:
    std::string name;
    btScalar eleapsedTime;
    bool running;
    
    static NameManager nameManager;
};

#endif