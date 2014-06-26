//
//  MonoWheelLateral.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__MonoWheelLateral__
#define __Stonefish__MonoWheelLateral__

#include "Controller.h"
#include "FakeIMU.h"
#include "FakeRotaryEncoder.h"
#include "Current.h"
#include "DCMotor.h"
#include "Polynomials.h"

class MonoWheelLateral : public Controller
{
public:
    MonoWheelLateral(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* leverEnc, Current* leverCurrent, DCMotor* leverMotor, btScalar frequency = btScalar(-1.));
    ~MonoWheelLateral();
    
    void Reset();
    ControllerType getType();
    
private:
    void Tick(btScalar dt);
    
    FakeIMU* imu;
    FakeRotaryEncoder* enc;
    Current* current;
    DCMotor* motor;
    PolySurface* LF[5];
};

#endif
