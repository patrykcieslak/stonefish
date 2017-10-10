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
#include "OpenGLContent.h"

typedef enum {ACTUATOR_MOTOR, ACTUATOR_THRUSTER, ACTUATOR_LIGHT} ActuatorType;

//abstract class
class Actuator
{
public:
    Actuator(std::string uniqueName);
    virtual ~Actuator();
    
    virtual void Update(btScalar dt) = 0;
    virtual std::vector<Renderable> Render();
    
    virtual ActuatorType getType() = 0;
    std::string getName();
    
private:
    std::string name;
    static NameManager nameManager;
};

#endif 
