//
//  ServoController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ServoController__
#define __Stonefish_ServoController__

#include "controllers/FeedbackController.h"

namespace sf
{
    class RotaryEncoder;
    class Motor;
    
    //! Servo controller with PID.
    class ServoController : public FeedbackController
    {
    public:
        ServoController(std::string uniqueName, Motor* m, RotaryEncoder* e, Scalar maxOutput, Scalar frequency = Scalar(-1));
        ~ServoController();
        
        void Reset();
        
        void SetPosition(Scalar pos);
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
        Scalar targetPos;
        Scalar lastError;
        Scalar integratedError;
    };
}

#endif
