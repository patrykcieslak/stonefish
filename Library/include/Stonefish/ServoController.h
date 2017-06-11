//
//  ServoController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ServoController__
#define __Stonefish_ServoController__

#include "FeedbackController.h"
#include "Motor.h"
#include "RotaryEncoder.h"

/*! Servo controller with PID */
class ServoController : public FeedbackController
{
public:
    ServoController(std::string uniqueName, Motor* m, RotaryEncoder* e, btScalar maxOutput, btScalar frequency = btScalar(-1.));
    ~ServoController();
    
    void Reset();
    
    void SetPosition(btScalar pos);
    void SetGains(btScalar P, btScalar I, btScalar D, btScalar ILimit);
    
private:
    void Tick(btScalar dt);
    
    RotaryEncoder* encoder;
	Motor* motor;
    btScalar maxCtrl;
    btScalar gainP;
    btScalar gainI;
    btScalar gainD;
	btScalar limitI;
    
    //state
    btScalar targetPos;
    btScalar lastError;
    btScalar integratedError;
};


#endif
