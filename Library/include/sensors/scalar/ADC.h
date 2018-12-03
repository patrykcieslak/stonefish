//
//  ADC.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ADC__
#define __Stonefish_ADC__

#include "StonefishCommon.h"

namespace sf
{
    //!
    class ADC
    {
    public:
        ADC(unsigned short resolution, Scalar plusRefVoltage, Scalar minusRefVoltage = Scalar(0));
        
        Scalar MeasureVoltage(Scalar Vin);
        
    private:
        unsigned short bits;
        Scalar pRefV;
        Scalar mRefV;
    };
}

#endif
