//
//  Gyroscope.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Gyroscope.h"

using namespace sf;

Gyroscope::Gyroscope(std::string uniqueName, Scalar rangeMin, Scalar rangeMax, Scalar sensitivity, Scalar zeroVoltage, Scalar driftSpeed, Scalar noisePSD, ADC* adc, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
    
    range[0] = rangeMin;
    range[1] = rangeMax;
    sens = sensitivity;
    drift = driftSpeed;

    zeroV = zeroVoltage;
    this->noisePSD = noisePSD;
    this->adc = adc;
}

void Gyroscope::Reset()
{
    accumulatedDrift = 0;
    ScalarSensor::Reset();
}

void Gyroscope::InternalUpdate(Scalar dt)
{
    //calculate transformation from global to gyro frame
    Matrix3 toGyroFrame = getSensorFrame().inverse().getBasis();
    
    //get angular velocity
    Vector3 actualAV = attach->getAngularVelocity();
    actualAV = toGyroFrame * actualAV;
    
    //select axis Z
    Scalar av = actualAV.getZ();
    
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
