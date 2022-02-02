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
//  USBL.h
//  Stonefish
//
//  Created by Patryk Cieślak on 25/02/2020.
//  Copyright (c) 2020-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_USBL__
#define __Stonefish_USBL__

#include "comms/AcousticModem.h"
#include <random>

namespace sf
{
    struct BeaconInfo
    {
        Quaternion localOri; // Orientation of the USBL in the world frame (internal IMU)
        Scalar localDepth;   // Depth measured by the USBL (internal pressure sensor) 
        Scalar t;            // Roundtrip acoustic signal travel time
        Vector3 relPos;      // Position of beacon in the USBL frame
        Scalar elevation;    // Elevation angle computed from beacon response
        Scalar azimuth;      // Azimuth angle computed from beacon response
        Scalar range;        // Distance to the beacon
    };

    //! An abstract class representing a USBL.
    class USBL : public AcousticModem
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
        USBL(std::string uniqueName, uint64_t deviceId, Scalar minVerticalFOVDeg, Scalar maxVerticalFOVDeg, Scalar operatingRange);
        
        //! A method performing internal comm state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to enable the auto pinging of connected transponder to monitor its position.
        /*!
         \param rate how often the ping should be sent (0 for continuous mode) [Hz]
         */
        void EnableAutoPing(Scalar rate = Scalar(0));
        
        //! A method to diable the autp pinging funtion.
        void DisableAutoPing();
           
        //! A method to get the current information about the acoustic beacons.
        std::map<uint64_t, BeaconInfo>& getBeaconInfo(); 

        //! A method returning the type of the comm.
        CommType getType() const;
       
    protected:
        virtual void ProcessMessages() = 0;

        bool ping;
        Scalar pingRate;
        Scalar pingTime;
        std::map<uint64_t, BeaconInfo> beacons;
        bool noise;
        
        static std::random_device randomDevice;
        static std::mt19937 randomGenerator;
    };
}
    
#endif
