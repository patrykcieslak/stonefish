//
//  SpeedController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 16/09/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "controllers/SpeedController.h"

SpeedController::SpeedController(std::string uniqueName, Motor* m, RotaryEncoder* e, btScalar maxOutput, btScalar frequency) : FeedbackController(uniqueName, 1, frequency)
{
    motor = m;
    encoder = e;
    maxCtrl = maxOutput;
    gainP = btScalar(1.);
    gainI = btScalar(0.);
    gainD = btScalar(0.);
    
    Reset();
}

SpeedController::~SpeedController()
{
}

void SpeedController::SetSpeed(btScalar speed)
{
    setReferenceValue(0, UnitSystem::SetAngularVelocity(speed));
}

void SpeedController::SetGains(btScalar P, btScalar I, btScalar D, btScalar ILimit)
{
    gainP = P;
    gainI = I;
    gainD = D;
	limitI = ILimit;
}

void SpeedController::Reset()
{
    setReferenceValue(0, btScalar(0.));
    lastError = btScalar(0.);
    integratedError = btScalar(0.);
}

void SpeedController::Tick(btScalar dt)
{
    //get desired servo position
    std::vector<btScalar> ref = getReferenceValues();
    
    //get measurements
    Sample encSample = encoder->getLastSample();
    
    //calculate error
    btScalar error = ref[0] - encSample.getValue(1);
    
	//integrate and limit
	integratedError += error * dt;
	integratedError = integratedError > limitI ? limitI : (integratedError < -limitI ? -limitI : integratedError);
	
    btScalar derivativeError = (error - lastError)/dt;
    lastError = error;
    
    //calculate and apply control
    btScalar control = gainP * error + gainI * integratedError + gainD * derivativeError;
    control = control > maxCtrl ? maxCtrl : (control < -maxCtrl ? -maxCtrl : control);
	output = control;
	
    motor->setIntensity(output);
}
