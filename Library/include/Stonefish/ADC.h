//
//  ADC.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ADC__
#define __Stonefish_ADC__

#include "common.h"

class ADC
{
public:
    ADC(ushort resolution, btScalar plusRefVoltage, btScalar minusRefVoltage = btScalar(0.));
    
    btScalar MeasureVoltage(btScalar Vin);
    
private:
    ushort bits;
    btScalar pRefV;
    btScalar mRefV;
};



#endif
