//
//  Gyroscope.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Gyroscope.h"

using namespace sf;

Gyroscope::Gyroscope(std::string uniqueName, btScalar rangeMin, btScalar rangeMax, btScalar sensitivity, btScalar zeroVoltage, btScalar driftSpeed, btScalar noisePSD, ADC* adc, btScalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
    
    range[0] = UnitSystem::SetAngularVelocity(rangeMin);
    range[1] = UnitSystem::SetAngularVelocity(rangeMax);
    sens = UnitSystem::SetAngularVelocity(sensitivity);
    drift = UnitSystem::SetAngle(driftSpeed);

    zeroV = zeroVoltage;
    this->noisePSD = noisePSD;
    this->adc = adc;
}

void Gyroscope::Reset()
{
    accumulatedDrift = 0;
    ScalarSensor::Reset();
}

void Gyroscope::InternalUpdate(btScalar dt)
{
    //calculate transformation from global to gyro frame
    btMatrix3x3 toGyroFrame = getSensorFrame().inverse().getBasis();
    
    //get angular velocity
    btVector3 actualAV = attach->getAngularVelocity();
    actualAV = toGyroFrame * actualAV;
    
    //select axis Z
    btScalar av = actualAV.getZ();
    
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
