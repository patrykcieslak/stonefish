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
//  USBLSimple.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 25/02/2020.
//  Copyright (c) 2020-2021 Patryk Cieslak. All rights reserved.
//

#include "comms/USBLSimple.h"

namespace sf
{
        
USBLSimple::USBLSimple(std::string uniqueName, uint64_t deviceId, Scalar minVerticalFOVDeg, Scalar maxVerticalFOVDeg, Scalar operatingRange) 
           : USBL(uniqueName, deviceId, minVerticalFOVDeg, maxVerticalFOVDeg, operatingRange)
{
    rangeRes = Scalar(0);
    angleRes = Scalar(0);
}
    
void USBLSimple::setNoise(Scalar rangeDev, Scalar horizontalAngleDevDeg, Scalar verticalAngleDevDeg)
{
    noiseRange = std::normal_distribution<Scalar>(Scalar(0), btFabs(rangeDev));
    noiseHAngle = std::normal_distribution<Scalar>(Scalar(0), btFabs(horizontalAngleDevDeg)/Scalar(180)*M_PI);
    noiseVAngle = std::normal_distribution<Scalar>(Scalar(0), btFabs(verticalAngleDevDeg)/Scalar(180)*M_PI);
    noise = true;
}

void USBLSimple::setResolution(Scalar range, Scalar angleDeg)
{
    rangeRes = btFabs(range);
    angleRes = btFabs(angleDeg)/Scalar(180)*M_PI;
}

void USBLSimple::ProcessMessages()
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

            //Populate beacon info structure
            BeaconInfo b;
            b.t = t;
            b.relPos = pos;
            b.azimuth = hAngle;
            b.elevation = M_PI_2 - vAngle;
            b.range = slantRange;
            b.localDepth = dO.getZ();
            b.localOri = dT.getRotation();
            beacons[msg->source] = b;

            newDataAvailable = true;
        }
        
        delete msg;
    }
}

}