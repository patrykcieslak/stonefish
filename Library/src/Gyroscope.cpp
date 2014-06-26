//
//  Gyroscope.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Gyroscope.h"

Gyroscope::Gyroscope(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, AxisType senseAxis, btScalar rangeMin, btScalar rangeMax, btScalar sensitivity, btScalar zeroVoltage, btScalar driftSpeed, btScalar noisePSD, ADC* adc, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency, historyLength)
{
    solid = attachment;
    relToSolid = relFrame;
    axis = senseAxis;
    range[0] = rangeMin;
    range[1] = rangeMax;
    sens = sensitivity;
    zeroV = zeroVoltage;
    drift = driftSpeed;
    this->noisePSD = noisePSD;
    this->adc = adc;
    
    Reset();
}

void Gyroscope::Reset()
{
    Sensor::Reset();
    accumulatedDrift = 0;
}

void Gyroscope::InternalUpdate(btScalar dt)
{
    //calculate transformation from global to gyro frame
    btMatrix3x3 toGyroFrame = relToSolid.getBasis().inverse() * solid->getRigidBody()->getCenterOfMassTransform().getBasis().inverse();
    
    //get angular velocity
    btVector3 actualAV = solid->getRigidBody()->getAngularVelocity();
    actualAV = toGyroFrame * actualAV;
    
    //select axis and convert to external unit system
    btScalar av = actualAV[axis];
    av = UnitSystem::GetAngle(av);
    
    //add limits/noise/nonlinearity/drift
    accumulatedDrift += drift * dt;
    av += accumulatedDrift;
    av = av < range[0] ? range[0] : (av > range[1] ? range[1] : av);
    
    //put through ADC
    av = (adc->MeasureVoltage(av * sens + zeroV) - zeroV) / sens; //sensitivity V/(deg/s)
    
    //save sample
    Sample s(1, &av);
    AddSampleToHistory(s);
}

unsigned short Gyroscope::getNumOfDimensions()
{
    return 1;
}