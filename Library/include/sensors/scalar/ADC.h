//
//  ADC.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/03/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ADC__
#define __Stonefish_ADC__

#include "StonefishCommon.h"

namespace sf
{
    //! A class implementing a model of an analog-digital converter.
    class ADC
    {
    public:
        //! A constructor.
        /*!
         \param resolution the value of the ADC resoultion [bit]
         \param plusRefVoltage the max reference voltage [V]
         \param minusRefVoltage the min reference voltage [V]
         */
        ADC(unsigned short resolution, Scalar plusRefVoltage, Scalar minusRefVoltage = Scalar(0));
        
        //! A method returning the analog voltage converted to digital value.
        /*!
         \param Vin the input voltage (analog) [V]
         \return the digital reading [V]
         */
        Scalar MeasureVoltage(Scalar Vin);
        
    private:
        unsigned short bits;
        Scalar pRefV;
        Scalar mRefV;
    };
}

#endif
