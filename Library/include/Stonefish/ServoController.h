//
//  ServoController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ServoController__
#define __Stonefish_ServoController__

#include "Controller.h"
#include "DCMotor.h"
#include "FakeRotaryEncoder.h"

class ServoController : public Controller
{
public:
    ServoController(std::string uniqueName, DCMotor* m, FakeRotaryEncoder* e, btScalar maxVoltage, btScalar frequency);
    ~ServoController();
    
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
