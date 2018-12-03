//
//  Gyroscope.h
//  Stonefish
//
//  Created by Patryk Cieslak on 29/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Gyroscope__
#define __Stonefish_Gyroscope__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    class ADC;
    
    //!
    class Gyroscope : public LinkSensor
    {
    public:
        Gyroscope(std::string uniqueName, Scalar rangeMin, Scalar rangeMax, Scalar sensitivity, Scalar zeroVoltage, Scalar driftSpeed, Scalar noisePSD, ADC* adc, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        void InternalUpdate(Scalar dt);
        void Reset();
        
    private:
        Scalar range[2];
        Scalar sens;
        Scalar zeroV;
        Scalar drift;
        Scalar noisePSD;
        ADC* adc;
        
        //temporary
        Scalar accumulatedDrift;
    };
}

#endif
