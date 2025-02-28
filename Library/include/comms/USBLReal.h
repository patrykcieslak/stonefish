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
//  USBLReal.h
//  Stonefish
//
//  Created by Patryk Cieślak on 21/12/2020.
//  Copyright (c) 2020-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_USBLReal__
#define __Stonefish_USBLReal__

#include "comms/USBL.h"

namespace sf
{
    //! A class representing a realistic USBL model.
    class USBLReal : public USBL
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the comm device
         \param deviceId an identification code of the device
         \param minVerticalFOVDeg the minimum vertical angle of radiation pattern [deg]
         \param maxVerticalFOVDeg the maximum vertical angle of radiation pattern [deg]
         \param operatingRange the operating range [m]
         \param carrierFrequency the base frequency of the signal [Hz]
         \param baseline the distance between transducers forming one pair [m]
         */
        USBLReal(std::string uniqueName, uint64_t deviceId, Scalar minVerticalFOVDeg, Scalar maxVerticalFOVDeg, Scalar operatingRange,
                 Scalar carrierFrequency, Scalar baseline);
           
        //! A method used to set the noise characteristics of the device.
        /*!
         \param timeDev standard deviation of the TOF measurement [s]
         \param soundVelocityDev standard deviation of the sound velocity in water [m/s]
         \param phaseDev standard deviation of the phase measurement [1]
         \param baselineError the error in the distance between transducers forming a pair [m]
         \param depthDev standard deviation of the depth measurement [m]
         */
        void setNoise(Scalar timeDev, Scalar soundVelocityDev, Scalar phaseDev, Scalar baselineError, Scalar depthDev);
        
    protected:
        void ProcessMessages();
    
    private:
        Scalar CalcModel(Scalar R, Scalar theta);

        Scalar freq;
        Scalar bl;
        Scalar blError;
        std::normal_distribution<Scalar> noiseTime;
        std::normal_distribution<Scalar> noiseSV;
        std::normal_distribution<Scalar> noisePhase;
        std::normal_distribution<Scalar> noiseDepth;
    };
}
    
#endif
