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
//  USBL.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 25/02/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "comms/USBL.h"

namespace sf
{
    
std::random_device USBL::randomDevice;
std::mt19937 USBL::randomGenerator(randomDevice());
    
USBL::USBL(std::string uniqueName, uint64_t deviceId, Scalar horizontalFOVDeg, Scalar verticalFOVDeg, Scalar operatingRange) 
           : AcousticModem(uniqueName, deviceId, horizontalFOVDeg, verticalFOVDeg, operatingRange)
{
    ping = false;
    noise = false;
}
    
std::map<uint64_t, std::pair<Scalar, Vector3>>& USBL::getTransponderPositions()
{
    return transponderPos;
}

CommType USBL::getType() const
{
    return CommType::USBL;
}

void USBL::EnableAutoPing(Scalar rate)
{
    if(rate > Scalar(0))
    {
        pingTime = Scalar(0);
        pingRate = rate;
        ping = true;
    }
}
 
void USBL::DisableAutoPing()
{
    ping = false;
}

void USBL::InternalUpdate(Scalar dt)
{
    AcousticModem::InternalUpdate(dt);
    
    //Auto pinging with fixed rate
    if(ping && pingRate > Scalar(0))
    {
        pingTime += dt;
        Scalar invRate = Scalar(1)/pingRate;
        
        if(pingTime >= invRate)
        {
            SendMessage("PING");
            pingTime -= invRate;
        }
    }
}

}