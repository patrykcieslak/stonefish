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
//  ADC.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/ADC.h"

namespace sf
{

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
    
}
