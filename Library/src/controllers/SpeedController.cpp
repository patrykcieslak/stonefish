//
//  SpeedController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 16/09/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "controllers/SpeedController.h"

#include "actuators/Motor.h"
#include "sensors/scalar/RotaryEncoder.h"
#include "sensors/Sample.h"

namespace sf
{

SpeedController::SpeedController(std::string uniqueName, Motor* m, RotaryEncoder* e, Scalar maxOutput, Scalar frequency) : FeedbackController(uniqueName, 1, frequency)
{
    motor = m;
    encoder = e;
    maxCtrl = maxOutput;
    gainP = Scalar(1.);
    gainI = Scalar(0.);
    gainD = Scalar(0.);
    
    Reset();
}

void SpeedController::SetSpeed(Scalar speed)
{
    setReferenceValue(0, speed);
}

void SpeedController::SetGains(Scalar P, Scalar I, Scalar D, Scalar ILimit)
{
    gainP = P;
    gainI = I;
    gainD = D;
	limitI = ILimit;
}

void SpeedController::Reset()
{
    setReferenceValue(0, Scalar(0.));
    lastError = Scalar(0.);
    integratedError = Scalar(0.);
}

void SpeedController::Tick(Scalar dt)
{
    //get desired servo position
    std::vector<Scalar> ref = getReferenceValues();
    
    //get measurements
    Sample encSample = encoder->getLastSample();
    
    //calculate error
    Scalar error = ref[0] - encSample.getValue(1);
    
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

}
