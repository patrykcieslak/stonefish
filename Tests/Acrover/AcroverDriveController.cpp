//
//  AcroverDriveController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015 Patryk Cieslak. All rights reserved.
//

#include "AcroverDriveController.h"

#pragma mark Constructors
AcroverDriveController::AcroverDriveController(std::string uniqueName, DCMotor* m, RotaryEncoder* e, btScalar maxTorqueRef, btScalar maxVoltage, btScalar frequency) : FeedbackController(uniqueName, 1, frequency)
{
    motor = m;
    encoder = e;
    maxV = maxVoltage;
    maxT = maxTorqueRef;
    gainP = btScalar(1.);
    gainI = btScalar(0.);
    
    Reset();
}

#pragma mark - Destructor
AcroverDriveController::~AcroverDriveController()
{
}

#pragma mark - Accessors
void AcroverDriveController::setTorque(btScalar tau)
{
    tau = tau > maxT ? maxT : (tau < -maxT ? -maxT : tau);
    setReferenceValue(0, UnitSystem::SetTorque(tau));
}

void AcroverDriveController::SetGains(btScalar P, btScalar I)
{
    gainP = P;
    gainI = I;
}

#pragma mark - Methods
void AcroverDriveController::Reset()
{
    setReferenceValue(0, btScalar(0.));
    lastError = btScalar(0.);
    integratedError = btScalar(0.);
}

void AcroverDriveController::Tick(btScalar dt)
{
//    //get desired servo position
//    std::vector<btScalar> ref = getReferenceValues();
//    
//    //get measurements
//    Sample encSample = encoder->getLastSample();
//    
//    //estimate generated torque
//    btScalar torque = motor->getTorque();//motor->getKt() * motor->getGearRatio() * motor->getCurrent();
//    
//    //calculate error
//    btScalar error = ref[0] - torque;
//    integratedError += error * dt;
//    lastError = error;
//    
//    //calculate feedforward term
//    btScalar ff = 9.5493 * motor->getKe() * motor->getGearRatio() * encSample.getValue(1);
//    
//    //calculate and apply control
//    btScalar control = gainP * error + gainI * integratedError + ff;
//    control = control > maxV ? maxV : (control < -maxV ? -maxV : control);
//    
//    //printf("Torque: %1.5f FF: %1.5f Error: %1.5f Control: %1.5f\n", torque, ff, error, control);
//    
//    motor->setVoltage(control);
    
    //get desired torque
    std::vector<btScalar> ref = getReferenceValues();

    //get measurements
    Sample encSample = encoder->getLastSample();
    
    //calculate feedforward term
    btScalar ff = 9.5493 * motor->getKe() * motor->getGearRatio() * encSample.getValue(1);
    
    // calculate and apply control
    btScalar control = ref[0] * motor->getR() / (motor->getKt() * motor->getGearRatio()) + ff;
    control = control > maxV ? maxV : (control < -maxV ? -maxV : control);

    motor->setVoltage(control);
}