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
//  USBLSimple.h
//  Stonefish
//
//  Created by Patryk Cieślak on 25/02/2020.
//  Copyright (c) 2020-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_USBLSimple__
#define __Stonefish_USBLSimple__

#include "comms/USBL.h"

namespace sf
{
    //! A class representing a simplified USBL model.
    class USBLSimple : public USBL
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the comm device
         \param deviceId an identification code of the device
         \param minVerticalFOVDeg the minimum vertical angle of radiation pattern [deg]
         \param maxVerticalFOVDeg the maximum vertical angle of radiation pattern [deg]
         \param operatingRange the operating range [m]
         */
        USBLSimple(std::string uniqueName, uint64_t deviceId, Scalar minVerticalFOVDeg, Scalar maxVerticalFOVDeg, Scalar operatingRange);
        
        //! A method used to set the resolution of the measurements.
        /*!
         \param range the range measurement resolution [m]
         \param angleDeg the angle measurement resolution [deg]
         */
        void setResolution(Scalar range, Scalar angleDeg);

        //! A method used to set the noise characteristics of the device.
        /*!
         \param rangeDev standard deviation of the range measurement noise [m]
         \param horizontalAngleDevDeg standard deviation of the angle measurement noise [deg]
         \param verticalAngleDevDeg standard deviation of the angle measurement noise [deg]
         */
        void setNoise(Scalar rangeDev, Scalar horizontalAngleDevDeg, Scalar verticalAngleDevDeg);
        
    protected:
        void ProcessMessages();
    
    private:
        Scalar rangeRes;
        Scalar angleRes;
        std::normal_distribution<Scalar> noiseRange;
        std::normal_distribution<Scalar> noiseHAngle;
        std::normal_distribution<Scalar> noiseVAngle;
    };
}
    
#endif
