//
//  UnderwaterVehicle.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_UnderwaterVehicle__
#define __Stonefish_UnderwaterVehicle__

#include "UnitSystem.h"
#include "NameManager.h"
#include "OpenGLPipeline.h"
#include "SolidEntity.h"
#include "Sensor.h"
#include "Thruster.h"
#include "Manipulator.h"

class UnderwaterVehicle
{
public:
    UnderwaterVehicle(std::string uniqueName);
    virtual ~UnderwaterVehicle();
    
private:
    std::vector<SolidEntity*> body;
    std::vector<SolidEntity*> shell;
    std::vector<Sensor*> sensors;
    std::vector<Thruster*> thrusters;
    //TODO: fins
    std::vector<Manipulator*> manipulators;
};

#endif