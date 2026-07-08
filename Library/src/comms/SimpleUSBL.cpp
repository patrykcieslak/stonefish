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
//  SimpleUSBL.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 25/02/2020.
//  Copyright (c) 2020-2026 Patryk Cieslak. All rights reserved.
//

#include "comms/SimpleUSBL.h"

namespace sf
{
        
SimpleUSBL::SimpleUSBL(const std::string& uniqueName, uint64_t deviceId, Scalar minVerticalFOVDeg, Scalar maxVerticalFOVDeg, Scalar operatingRange) 
           : USBL(uniqueName, deviceId, minVerticalFOVDeg, maxVerticalFOVDeg, operatingRange)
{
    rangeRes_ = Scalar(0);
    angleRes_ = Scalar(0);
}
    
void SimpleUSBL::setNoise(Scalar rangeDev, Scalar horizontalAngleDevDeg, Scalar verticalAngleDevDeg)
{
    noiseRange_ = std::normal_distribution<Scalar>(Scalar(0), btFabs(rangeDev));
    noiseHAngle_ = std::normal_distribution<Scalar>(Scalar(0), btFabs(horizontalAngleDevDeg)/Scalar(180)*M_PI);
    noiseVAngle_ = std::normal_distribution<Scalar>(Scalar(0), btFabs(verticalAngleDevDeg)/Scalar(180)*M_PI);
    noise_ = true;
}

void SimpleUSBL::setResolution(Scalar range, Scalar angleDeg)
{
    rangeRes_ = btFabs(range);
    angleRes_ = btFabs(angleDeg)/Scalar(180)*M_PI;
}

void SimpleUSBL::ProcessMessages()
{
    std::shared_ptr<AcousticDataFrame> msg;
    std::string ack {"ACK"};
    std::vector<uint8_t> ackData(ack.begin(), ack.end());

    for(auto it = rxBuffer_.begin(); it != rxBuffer_.end(); ++it)
    {
        msg = std::static_pointer_cast<AcousticDataFrame>(*it);
        if(msg->data == ackData)
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
            if(noise_)
            {
                slantRange += noiseRange_(randomGenerator);
                hAngle += noiseHAngle_(randomGenerator);
                vAngle += noiseVAngle_(randomGenerator);
            }

            if(rangeRes_ > Scalar(0))
                slantRange -= btFmod(slantRange, rangeRes_); //Quantization

            if(angleRes_ > Scalar(0))
            {
                hAngle -= btFmod(hAngle, angleRes_); //Quantization
                vAngle -= btFmod(vAngle, angleRes_); //Quantization
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
            beacons_[msg->source] = b;

            newDataAvailable_ = true;
        }
    }

    // Standard processing of messages and removal of "ACK and "PING" messages
    AcousticModem::ProcessMessages();
}

}