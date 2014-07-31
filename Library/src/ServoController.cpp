//
//  ServoController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "ServoController.h"

#pragma mark Constructors
ServoController::ServoController(std::string uniqueName, DCMotor* m, RotaryEncoder* e, btScalar maxVoltage, btScalar frequency) : FeedbackController(uniqueName, 1, frequency)
{
    motor = m;
    encoder = e;
    maxV = maxVoltage;
    gainP = btScalar(1.);
    gainI = btScalar(0.);
    gainD = btScalar(0.);
    
    Reset();
}

#pragma mark - Destructor
ServoController::~ServoController()
{
}

#pragma mark - Accessors
void ServoController::SetPosition(btScalar pos)
{
    setReferenceValue(0, UnitSystem::SetAngle(pos));
}

void ServoController::SetGains(btScalar P, btScalar I, btScalar D)
{
    gainP = P;
    gainI = I;
    gainD = D;
}

#pragma mark - Methods
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
    integratedError += error * dt;
    btScalar derivativeError = (error - lastError)/dt;
    lastError = error;
    
    //calculate and apply control
    btScalar control = gainP * error + gainI * integratedError + gainD * derivativeError;
    control = control > maxV ? maxV : (control < -maxV ? -maxV : control);
    motor->setVoltage(control);
}