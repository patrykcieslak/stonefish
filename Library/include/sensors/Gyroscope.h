//
//  Gyroscope.h
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Gyroscope__
#define __Stonefish_Gyroscope__

#include "sensors/SimpleSensor.h"
#include "sensors/ADC.h"

class Gyroscope : public SimpleSensor
{
public:
    Gyroscope(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, AxisType senseAxis, btScalar rangeMin, btScalar rangeMax, btScalar sensitivity, btScalar zeroVoltage, btScalar driftSpeed, btScalar noisePSD, ADC* adc, btScalar frequency = btScalar(-1.), int historyLength = -1);
    
    void InternalUpdate(btScalar dt);
    void Reset();
    btTransform getSensorFrame();
    
private:
    SolidEntity* solid;
    AxisType axis;
    btScalar range[2];
    btScalar sens;
    btScalar zeroV;
    btScalar drift;
    btScalar noisePSD;
    ADC* adc;
    
    //temporary
    btScalar accumulatedDrift;
};


#endif
