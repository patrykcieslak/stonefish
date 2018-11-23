//
//  SpeedController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 16/09/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SpeedController__
#define __Stonefish_SpeedController__

#include "actuators/Motor.h"
#include "sensors/scalar/RotaryEncoder.h"
#include "controllers/FeedbackController.h"

namespace sf
{

/*! Speed controller with PID */
class SpeedController : public FeedbackController
{
public:
    SpeedController(std::string uniqueName, Motor* m, RotaryEncoder* e, btScalar maxOutput, btScalar frequency = btScalar(-1));
    ~SpeedController();
    
    void Reset();
    
    void SetSpeed(btScalar speed);
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
    btScalar targetSpeed;
    btScalar lastError;
    btScalar integratedError;
};
    
}

#endif
