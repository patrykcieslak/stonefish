//
//  ServoController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "controllers/ServoController.h"

using namespace sf;

ServoController::ServoController(std::string uniqueName, Motor* m, RotaryEncoder* e, Scalar maxOutput, Scalar frequency) : FeedbackController(uniqueName, 1, frequency)
{
    motor = m;
    encoder = e;
    maxCtrl = maxOutput;
    gainP = Scalar(1.);
    gainI = Scalar(0.);
    gainD = Scalar(0.);
    
    Reset();
}

ServoController::~ServoController()
{
}

void ServoController::SetPosition(Scalar pos)
{
    setReferenceValue(0, pos);
}

void ServoController::SetGains(Scalar P, Scalar I, Scalar D, Scalar ILimit)
{
    gainP = P;
    gainI = I;
    gainD = D;
	limitI = ILimit;
}

void ServoController::Reset()
{
    setReferenceValue(0, Scalar(0.));
    lastError = Scalar(0.);
    integratedError = Scalar(0.);
}

void ServoController::Tick(Scalar dt)
{
    //get desired servo position
    std::vector<Scalar> ref = getReferenceValues();
    
    //get measurements
    Sample encSample = encoder->getLastSample();
    
    //calculate error
    Scalar error = ref[0] - encSample.getValue(0);
    
	//integrate and limit
	integratedError += error * dt;
	integratedError = integratedError > limitI ? limitI : (integratedError < -limitI ? -limitI : integratedError);
	
    Scalar derivativeError = (error - lastError)/dt;
    lastError = error;
    
    //calculate and apply control
    Scalar control = gainP * error + gainI * integratedError + gainD * derivativeError;
    control = control > maxCtrl ? maxCtrl : (control < -maxCtrl ? -maxCtrl : control);
	output = control;
		
    motor->setIntensity(output);
}
