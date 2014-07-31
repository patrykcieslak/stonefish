//
//  MonoWheelLateral.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__MonoWheelLateral__
#define __Stonefish__MonoWheelLateral__

#include "FeedbackController.h"
#include "FakeIMU.h"
#include "FakeRotaryEncoder.h"
#include "Current.h"
#include "DCMotor.h"
#include "Polynomials.h"

class MonoWheelLateral : public FeedbackController
{
public:
    MonoWheelLateral(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* leverEnc, FakeRotaryEncoder* wheelEnc, Current* leverCurrent, DCMotor* leverMotor, btScalar maxVoltage, btScalar frequency = btScalar(-1.));
    ~MonoWheelLateral();
    
    void Reset();
    
    void setDesiredTilt(btScalar tilt);
    
private:
    void Tick(btScalar dt);
    
    FakeIMU* imu;
    FakeRotaryEncoder* lEnc;
    FakeRotaryEncoder* wEnc;
    Current* current;
    DCMotor* motor;
    btScalar tyreRadius;
    btScalar maxV;
    
    PolySurface* LF[5];
    std::vector<btScalar> gains;
};

#endif
