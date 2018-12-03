//
//  DCMotor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DCMotor__
#define __Stonefish_DCMotor__

#include "actuators/Motor.h"

namespace sf
{
    //!
    class DCMotor : public Motor
    {
    public:
        DCMotor(std::string uniqueName, Scalar motorR, Scalar motorL, Scalar motorKe, Scalar motorKt, Scalar friction);
        
        //Motor
        void Update(Scalar dt);
        void setIntensity(Scalar value);
        Scalar getTorque();
        Scalar getAngle();
        Scalar getAngularVelocity();
        
        //DC motor
        void SetupGearbox(bool enable, Scalar ratio, Scalar efficiency);
        void setVoltage(Scalar volt);
        Scalar getCurrent();
        Scalar getVoltage();
        Scalar getKe();
        Scalar getKt();
        Scalar getGearRatio();
        Scalar getR();
        
    private:
        //inputs
        Scalar V;
        
        //states
        Scalar I;
        
        //motor params
        Scalar R;
        Scalar L;
        Scalar Ke;
        Scalar Kt;
        Scalar B;
        bool gearEnabled;
        Scalar gearRatio;
        Scalar gearEff;
        
        //integral
        Scalar lastVoverL;
    };
}

#endif
