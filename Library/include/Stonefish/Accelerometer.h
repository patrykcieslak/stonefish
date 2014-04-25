//
//  Accelerometer.h
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Accelerometer__
#define __Stonefish_Accelerometer__

#include "Sensor.h"
#include "ADC.h"
#include "SolidEntity.h"

class Accelerometer : public Sensor
{
public:
    Accelerometer(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, AxisType senseAxis, btScalar rangeMin, btScalar rangeMax, btScalar sensitivity, btScalar zeroVoltage, btScalar noisePSD, ADC* adc, bool measuresInG = true, unsigned int historyLength = 1);
    
    void Reset();
    void Update(btScalar dt);
    
    unsigned short getNumOfDimensions();
    
private:
    //parameters
    SolidEntity* solid;
    btTransform relToSolid;
    AxisType axis;
    btScalar range[2];
    btScalar sens;
    btScalar zeroV;
    btScalar noisePSD;
    ADC* adc;
    bool G;
    
    //temporary
    btVector3 lastV;
};

#endif
