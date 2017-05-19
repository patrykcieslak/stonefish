//
//  Manipulator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Manipulator__
#define __Stonefish_Manipulator__

#include "UnitSystem.h"
#include "NameManager.h"
#include "OpenGLPipeline.h"
#include "FeatherstoneEntity.h"
#include "Gripper.h"

class Manipulator
{
public:
    Manipulator(std::string uniqueName);
    virtual ~Manipulator();
    
private:
    FeatherstoneEntity* links;
    Gripper* gripper;
};

#endif