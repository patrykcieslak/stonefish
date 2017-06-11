//
//  ServoController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "ServoController.h"

ServoController::ServoController(std::string uniqueName, Motor* m, RotaryEncoder* e, btScalar maxOutput, btScalar frequency) : FeedbackController(uniqueName, 1, frequency)
{
    motor = m;
    encoder = e;
    maxCtrl = maxOutput;
    gainP = btScalar(1.);
    gainI = btScalar(0.);
    gainD = btScalar(0.);
    
    Reset();
}

ServoController::~ServoController()
{
}

void ServoController::SetPosition(btScalar pos)
{
    setReferenceValue(0, UnitSystem::SetAngle(pos));
}

void ServoController::SetGains(btScalar P, btScalar I, btScalar D, btScalar ILimit)
{
    gainP = P;
    gainI = I;
    gainD = D;
	limitI = ILimit;
}

void ServoController::Reset()
{
    setReferenceValue(0, btScalar(0.));
    lastError = btScalar(0.);
    integratedError = btScalar(0.);
}

void ServoController::Tick(btScalar dt)
{
    //get desired servo position
    std::vector<btScalar> ref = getReferenceValues();
    
    //get measurements
    Sample encSample = encoder->getLastSample();
    
    //calculate error
    btScalar error = ref[0] - encSample.getValue(0);
    
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