//
//  Gyroscope.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Gyroscope.h"

#pragma mark Constructors
Gyroscope::Gyroscope(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, AxisType senseAxis, btScalar rangeMin, btScalar rangeMax, btScalar sensitivity, btScalar zeroVoltage, btScalar driftSpeed, btScalar noisePSD, ADC* adc, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    solid = attachment;
    relToSolid = relFrame;
    axis = senseAxis;
    
    std::string axisName[3] = {"X", "Y", "Z"};
    channels.push_back(SensorChannel("Angular velocity " + axisName[axis], QUANTITY_ANGULAR_VELOCITY));
    
    range[0] = UnitSystem::SetAngularVelocity(rangeMin);
    range[1] = UnitSystem::SetAngularVelocity(rangeMax);
    sens = UnitSystem::SetAngularVelocity(sensitivity);
    drift = UnitSystem::SetAngle(driftSpeed);

    zeroV = zeroVoltage;
    this->noisePSD = noisePSD;
    this->adc = adc;
}

#pragma mark - Methods
void Gyroscope::Reset()
{
    accumulatedDrift = 0;
    
    Sensor::Reset();
}

void Gyroscope::InternalUpdate(btScalar dt)
{
    //calculate transformation from global to gyro frame
    btMatrix3x3 toGyroFrame = relToSolid.getBasis().inverse() * solid->getTransform().getBasis().inverse();
    
    //get angular velocity
    btVector3 actualAV = solid->getAngularVelocity();
    actualAV = toGyroFrame * actualAV;
    
    //select axis
    btScalar av = actualAV[axis];
    
    //add limits/noise/nonlinearity/drift
    accumulatedDrift += drift * dt;
    av += accumulatedDrift;
    av = av < range[0] ? range[0] : (av > range[1] ? range[1] : av);
    
    //put through ADC
    av = (adc->MeasureVoltage(av * sens + zeroV) - zeroV) / sens; //sensitivity V/(rad/s)
    
    //save sample
    Sample s(1, &av);
    AddSampleToHistory(s);
}