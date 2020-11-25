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
    rangeRes = Scalar(0);
    angleRes = Scalar(0);
}
    
void USBL::setNoise(Scalar rangeDev, Scalar horizontalAngleDevDeg, Scalar verticalAngleDevDeg)
{
    noiseRange = std::normal_distribution<Scalar>(Scalar(0), btFabs(rangeDev));
    noiseHAngle = std::normal_distribution<Scalar>(Scalar(0), btFabs(horizontalAngleDevDeg)/Scalar(180)*M_PI);
    noiseVAngle = std::normal_distribution<Scalar>(Scalar(0), btFabs(verticalAngleDevDeg)/Scalar(180)*M_PI);
    noise = true;
}

void USBL::setResolution(Scalar range, Scalar angleDeg)
{
    rangeRes = btFabs(range);
    angleRes = btFabs(angleDeg)/Scalar(180)*M_PI;
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
            Scalar slantRange = msg->travelled/Scalar(2); //Distance to node is hald of the full travelled distance
            Scalar t = msg->timeStamp + slantRange/SOUND_VELOCITY_WATER;
            
            //Find ranging angles
            Scalar d = Vector3(dir.getX(), dir.getY(), Scalar(0)).length();
            Scalar hAngle = atan2(dir.getY(), dir.getX());
            Scalar vAngle = atan2(d, dir.getZ());
        
            //Apply noise and quantization
            if(noise)
            {
                slantRange += noiseRange(randomGenerator);
                hAngle += noiseHAngle(randomGenerator);
                vAngle += noiseVAngle(randomGenerator);
            }

            if(rangeRes > Scalar(0))
                slantRange -= btFmod(slantRange, rangeRes); //Quantization

            if(angleRes > Scalar(0))
            {
                hAngle -= btFmod(hAngle, angleRes); //Quantization
                vAngle -= btFmod(vAngle, angleRes); //Quantization
            }
            
            //Calculate receiver position
            Vector3 pos;
            dir.setX(btCos(hAngle) * btSin(vAngle));
            dir.setY(btSin(hAngle) * btSin(vAngle));
            dir.setZ(btCos(vAngle));
            dir.normalize();
            pos = dir * slantRange;

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