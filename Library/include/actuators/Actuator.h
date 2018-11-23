//
//  Actuator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/8/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Actuator__
#define __Stonefish_Actuator__

#include "graphics/OpenGLContent.h"

namespace sf
{

typedef enum {ACTUATOR_JOINT = 0, ACTUATOR_LINK} ActuatorType;

//abstract
class Actuator
{
public:
    Actuator(std::string uniqueName);
    virtual ~Actuator();
    
    virtual void Update(btScalar dt) = 0;
    virtual ActuatorType getType() = 0;
    
    virtual std::vector<Renderable> Render();
    std::string getName();
    
private:
    std::string name;
};
    
}

#endif 
