//
//  ServoController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ServoController__
#define __Stonefish_ServoController__

#include "FeedbackController.h"
#include "DCMotor.h"
#include "RotaryEncoder.h"

/*! DC servo controller with PID */
class ServoController : public FeedbackController
{
public:
    ServoController(std::string uniqueName, DCMotor* m, RotaryEncoder* e, btScalar maxVoltage, btScalar frequency = btScalar(-1.));
    ~ServoController();
    
    void Reset();
    
    void SetPosition(btScalar pos);
    void SetGains(btScalar P, btScalar I, btScalar D);
    
private:
    void Tick(btScalar dt);
    
    RotaryEncoder* encoder;
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
