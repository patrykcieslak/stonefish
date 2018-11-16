//
//  ADC.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "sensors/ADC.h"

ADC::ADC(unsigned short resolution, btScalar plusRefVoltage, btScalar minusRefVoltage)
{
    bits = resolution;
    pRefV = plusRefVoltage;
    mRefV = minusRefVoltage;
}

btScalar ADC::MeasureVoltage(btScalar Vin)
{
    if(Vin <= mRefV)
        return mRefV;
    else if(Vin >= pRefV)
        return pRefV;
    else
    {
        btScalar Vrange = pRefV - mRefV;
        btScalar frac = (Vin - mRefV) / Vrange;
        return btScalar(trunc(frac * btScalar((1 << bits) - 1))) / btScalar((1 << bits) - 1) * Vrange; //quantization
    }
}
