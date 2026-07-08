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
//  RealUSBL.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/12/2020.
//  Copyright (c) 2020-2026 Patryk Cieslak. All rights reserved.
//

#include "comms/RealUSBL.h"

namespace sf
{

RealUSBL::RealUSBL(const std::string& uniqueName, uint64_t deviceId, Scalar minVerticalFOVDeg, Scalar maxVerticalFOVDeg, Scalar operatingRange,
             Scalar carrierFrequency, Scalar baseline) 
           : USBL(uniqueName, deviceId, minVerticalFOVDeg, maxVerticalFOVDeg, operatingRange)
{
    freq_ = carrierFrequency;
    bl_ = baseline;
    blError_ = Scalar(0);
}
    
void RealUSBL::setNoise(Scalar timeDev, Scalar soundVelocityDev, Scalar phaseDev, Scalar baselineError, Scalar depthDev)
{
    noiseTime_ = std::normal_distribution<Scalar>(Scalar(0), btFabs(timeDev));
    noiseSV_ = std::normal_distribution<Scalar>(Scalar(0), btFabs(soundVelocityDev));
    noisePhase_ = std::normal_distribution<Scalar>(Scalar(0), btFabs(phaseDev));
    noiseDepth_ = std::normal_distribution<Scalar>(Scalar(0), btFabs(depthDev));
    blError_ = baselineError;
    noise_ = true;
}

void RealUSBL::ProcessMessages()
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
            Scalar slantRange = msg->travelled/Scalar(2); //Distance to node is half of the full travelled distance
            Scalar t = msg->timeStamp + slantRange/SOUND_VELOCITY_WATER;
            
            //Find plane coordinates
            Scalar xLocal = CalcModel(slantRange, btAngle(dir, VX()));
            Scalar yLocal = CalcModel(slantRange, btAngle(dir, VY()));
            
            //Find depth coordinate
            Scalar zGlobal = cO.getZ();
            if(noise_)
            {
                zGlobal += noiseDepth_(randomGenerator); //Transmitter
                zGlobal -= noiseDepth_(randomGenerator); //Receiver
            }
            
            //Update position in the transponder and in the USBL
            Vector3 worldPos = dT * Vector3(xLocal, yLocal, 0);
            worldPos.setZ(zGlobal);
            cNode->UpdatePosition(worldPos, true);
            
            //Populate beacon info structure
            Scalar d = Vector3(dir.getX(), dir.getY(), Scalar(0)).length();
            Scalar hAngle = atan2(dir.getY(), dir.getX());
            Scalar vAngle = atan2(d, dir.getZ());
        
            BeaconInfo b;
            b.t = t;
            b.relPos = dT.inverse() * worldPos;
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

Scalar RealUSBL::CalcModel(Scalar R, Scalar theta)
{
    Scalar result = R * btCos(theta);
    if(noise_)
    {
        result += R * btCos(theta) * (noiseTime_(randomGenerator) + noiseSV_(randomGenerator) - blError_/bl_);
        result += R * SOUND_VELOCITY_WATER/freq_ * noisePhase_(randomGenerator)/(Scalar(2.0*M_PI) * bl_);  
    }
    return result;
}

}