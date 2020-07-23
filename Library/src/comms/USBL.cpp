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
    
void USBL::setNoise(Scalar rangeDev, Scalar angleDevDeg, Scalar nedDev, Scalar depthDev)
{
    noiseRange = std::normal_distribution<Scalar>(Scalar(0), fabs(rangeDev));
    noiseAngle = std::normal_distribution<Scalar>(Scalar(0), fabs(angleDevDeg)/Scalar(180)*M_PI);
    noiseNED = std::normal_distribution<Scalar>(Scalar(0), fabs(nedDev));
    noiseDepth = std::normal_distribution<Scalar>(Scalar(0), fabs(depthDev));
    noise = true;
}

std::map<uint64_t, std::pair<Scalar, Vector3>>& USBL::getTransponderPositions()
{
    return transponderPos;
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

void USBL::ProcessMessages()
{
    AcousticDataFrame* msg;
    while((msg = (AcousticDataFrame*)ReadMessage()) != nullptr)
    {
        if(msg->data == "ACK")
        {  
            //Get message data
            AcousticModem* cNode = getNode(msg->source);
            Vector3 cO = msg->txPosition;
            Transform dT = getDeviceFrame();
            Vector3 dO = dT.getOrigin();
            Vector3 dir = getDeviceFrame().getBasis().inverse() * ((cO - dO).normalized()); //Direction in device frame
            Scalar distance = msg->travelled/Scalar(2); //Distance to node is hald of the full travelled distance
            Scalar t = msg->timeStamp + distance/SOUND_VELOCITY_WATER;
            
            //Find ranging angles
            Scalar d = Vector3(dir.getX(), dir.getY(), Scalar(0)).length();
            Scalar vAngle = atan2(d, dir.getZ());
            Scalar hAngle = atan2(dir.getY(), dir.getX());
        
            //Find position of receiver
            Vector3 pos;
        
            if(noise)
            {
                dT.getOrigin().setX(dT.getOrigin().getX() + noiseNED(randomGenerator));
                dT.getOrigin().setY(dT.getOrigin().getY() + noiseNED(randomGenerator));
                dT.getOrigin().setZ(dT.getOrigin().getZ() + noiseDepth(randomGenerator));
                distance += noiseRange(randomGenerator);
                vAngle += noiseAngle(randomGenerator);
                hAngle += noiseAngle(randomGenerator);
            
                Vector3 corruptedDir;
                corruptedDir.setX(btCos(hAngle) * btSin(vAngle));
                corruptedDir.setY(btSin(hAngle) * btSin(vAngle));
                corruptedDir.setZ(btCos(vAngle));
                corruptedDir.normalize();
            
                pos = corruptedDir * distance;
            }
            else 
                pos = dir * distance;

            //Update position in the transponder and in the USBL
            Vector3 worldPos = dT * pos;
            cNode->UpdatePosition(worldPos, true);
            
            transponderPos[msg->source] = std::make_pair(t, pos);
            newDataAvailable = true;
        }
        
        delete msg;
    }
}

}