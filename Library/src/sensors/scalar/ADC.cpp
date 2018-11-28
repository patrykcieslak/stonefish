//
//  ADC.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/ADC.h"

using namespace sf;

ADC::ADC(unsigned short resolution, Scalar plusRefVoltage, Scalar minusRefVoltage)
{
    bits = resolution;
    pRefV = plusRefVoltage;
    mRefV = minusRefVoltage;
}

Scalar ADC::MeasureVoltage(Scalar Vin)
{
    if(Vin <= mRefV)
        return mRefV;
    else if(Vin >= pRefV)
        return pRefV;
    else
    {
        Scalar Vrange = pRefV - mRefV;
        Scalar frac = (Vin - mRefV) / Vrange;
        return Scalar(trunc(frac * Scalar((1 << bits) - 1))) / Scalar((1 << bits) - 1) * Vrange; //quantization
    }
}
