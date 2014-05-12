//
//  Servo.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Servo__
#define __Stonefish_Servo__

#include "Controller.h"
#include "DCMotor.h"
#include "FakeRotaryEncoder.h"

class Servo : public Controller
{
public:
    Servo(std::string uniqueName, DCMotor* m, FakeRotaryEncoder* e, btScalar maxVoltage, btScalar frequency);
    ~Servo();
    
    void SetPosition(btScalar pos);
    void Reset();
    void SetGains(btScalar P, btScalar I, btScalar D);
    
    ControllerType getType();
    
private:
    void Tick();
    
    FakeRotaryEncoder* encoder;
    DCMotor* motor;
    btScalar maxV;
    btScalar gainP;
    btScalar gainI;
    btScalar gainD;
    
    //state
    btScalar targetPos;
    btScalar lastError;
    btScalar integratedError;
};


#endif
