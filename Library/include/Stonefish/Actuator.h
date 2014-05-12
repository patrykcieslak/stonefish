//
//  Actuator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/8/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Actuator__
#define __Stonefish_Actuator__

#include "common.h"
#include "NameManager.h"

typedef enum {DC_MOTOR, STEPPER_MOTOR, LINEAR_DRIVE, THRUSTER, CONTROL_SURFACE} ActuatorType;

//abstract class
class Actuator
{
public:
    Actuator(std::string uniqueName);
    virtual ~Actuator();
    
    virtual void Update(btScalar dt) = 0;
    virtual btVector3 Render() = 0;
    
    void setRenderable(bool render);
    virtual ActuatorType getType() = 0;
    std::string getName();
    
    bool isRenderable();
    
private:
    std::string name;
    bool renderable;
    
    static NameManager nameManager;
};

#endif 
