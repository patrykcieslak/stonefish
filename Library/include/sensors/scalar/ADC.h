/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
