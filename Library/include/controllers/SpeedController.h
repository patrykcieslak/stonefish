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
    SpeedController(std::string uniqueName, Motor* m, RotaryEncoder* e, Scalar maxOutput, Scalar frequency = Scalar(-1));
    ~SpeedController();
    
    void Reset();
    
    void SetSpeed(Scalar speed);
    void SetGains(Scalar P, Scalar I, Scalar D, Scalar ILimit);
    
private:
    void Tick(Scalar dt);
    
    RotaryEncoder* encoder;
	Motor* motor;
    Scalar maxCtrl;
    Scalar gainP;
    Scalar gainI;
    Scalar gainD;
	Scalar limitI;
    
    //state
    Scalar targetSpeed;
    Scalar lastError;
    Scalar integratedError;
};
    
}

#endif
