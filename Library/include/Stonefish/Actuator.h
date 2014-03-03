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
#include "Joint.h"

//pure virtual class
class Actuator
{
public:
    typedef enum
    {
        DCMOTOR
    }
    ActuatorType;
    
    Actuator();
    virtual ~Actuator();
    
    void setRenderable(bool render);
    bool isRenderable();
    
    virtual void Render() = 0;
   	virtual ActuatorType getType() = 0;
    virtual void SetInput(btScalar* inputValues) = 0;
    virtual void Update(btScalar dt) = 0;
    
private:
    bool renderable;
};

#endif 
