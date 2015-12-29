//
//  AcroverDriveController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcroverDriveController__
#define __Stonefish__AcroverDriveController__

#include "FeedbackController.h"
#include "DCMotor.h"
#include "RotaryEncoder.h"

/*! DC motor torque controller with PI + Feefforward */
class AcroverDriveController : public FeedbackController
{
public:
    AcroverDriveController(std::string uniqueName, DCMotor* m, RotaryEncoder* e, btScalar maxTorqueRef, btScalar maxVoltage, btScalar frequency = btScalar(-1.));
    ~AcroverDriveController();
    
    void Reset();
    
    void setTorque(btScalar tau);
    void SetGains(btScalar P, btScalar I);
    
private:
    void Tick(btScalar dt);
    
    RotaryEncoder* encoder;
    DCMotor* motor;
    btScalar maxV;
    btScalar maxT;
    
    btScalar gainP;
    btScalar gainI;
    
    //state
    btScalar targetTorque;
    btScalar lastError;
    btScalar integratedError;
};


#endif /* defined(__Stonefish__AcroverDriveController__) */
