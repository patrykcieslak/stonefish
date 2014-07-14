//
//  Gyroscope.h
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Gyroscope__
#define __Stonefish_Gyroscope__

#include "Sensor.h"
#include "ADC.h"
#include "SolidEntity.h"

class Gyroscope : public Sensor
{
public:
    Gyroscope(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, AxisType senseAxis, btScalar rangeMin, btScalar rangeMax, btScalar sensitivity, btScalar zeroVoltage, btScalar driftSpeed, btScalar noisePSD, ADC* adc, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    
    void InternalUpdate(btScalar dt);
    void Reset();
    
private:
    //parameters
    SolidEntity* solid;
    btTransform relToSolid;
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
