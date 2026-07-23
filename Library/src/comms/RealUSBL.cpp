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

#include "core/DeviceFactory.h"

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

// Statics

ConstructInfo RealUSBL::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // Specs
    node.optional = false;
    node.attributes.insert({"min_vertical_fov", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"max_vertical_fov", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"range", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"frequency", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"baseline", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"specs", node});

    // Connect
    node.attributes.clear();
    node.optional = false;
    node.attributes.insert({"device_id", {ConstructInfoValueType::INT, false}});
    node.attributes.insert({"occlusion_test", {ConstructInfoValueType::BOOL, true}});
    info.nodes.insert({"connect", node});

    // Autoping (optional)
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"rate", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"autoping", node});

    // Noise
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"time_of_flight", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"sound_velocity", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"phase", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"depth", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"baseline_error", {ConstructInfoValueType::SCALAR, true}});
    info.nodes.insert({"noise", node});
    
    return info;
}

std::unique_ptr<RealUSBL> RealUSBL::Construct(const std::string& uniqueName, uint64_t deviceId, ConstructInfo& info)
{
    // Required
    Scalar minVerticalFov = std::get<Scalar>(info.nodes.at("specs").attributes.at("min_vertical_fov").value);
    Scalar maxVerticalFov = std::get<Scalar>(info.nodes.at("specs").attributes.at("max_vertical_fov").value);
    Scalar range = std::get<Scalar>(info.nodes.at("specs").attributes.at("range").value);
    Scalar frequency = std::get<Scalar>(info.nodes.at("specs").attributes.at("frequency").value);
    Scalar baseline = std::get<Scalar>(info.nodes.at("specs").attributes.at("baseline").value);

    // Construct
    std::unique_ptr<RealUSBL> comm = std::make_unique<RealUSBL>(uniqueName, deviceId, minVerticalFov, maxVerticalFov, 
        range, frequency, baseline);
    comm->Connect(std::get<int>(info.nodes.at("connect").attributes.at("device_id").value));
    
    // Optional
    bool occlusionTest {true};
    ConstructInfoValue& value = info.nodes.at("connect").attributes.at("occlusion_test");
    if (value.valid)
        occlusionTest = std::get<bool>(value.value);
    comm->setOcclusionTest(occlusionTest);
    
    value = info.nodes.at("autoping").attributes.at("rate");
    if (value.valid)
        comm->EnableAutoPing(std::get<Scalar>(value.value));

    // Noise (optional)
    Scalar tof {0.};
    Scalar soundVelocity {0.};
    Scalar phase {0.};
    Scalar depth {0.};
    Scalar baselineErr {0.};
    
    value = info.nodes.at("noise").attributes.at("time_of_flight");
    if (value.valid)
        tof = std::get<Scalar>(value.value);
    
    value = info.nodes.at("noise").attributes.at("sound_velocity");
    if (value.valid)
        soundVelocity = std::get<Scalar>(value.value);

    value = info.nodes.at("noise").attributes.at("phase");
    if (value.valid)
        phase = std::get<Scalar>(value.value);

    value = info.nodes.at("noise").attributes.at("depth");
    if (value.valid)
        depth = std::get<Scalar>(value.value);

    value = info.nodes.at("noise").attributes.at("baseline_error");
    if (value.valid)
        baselineErr = std::get<Scalar>(value.value);

    comm->setNoise(tof, soundVelocity, phase, baselineErr, depth);

    return comm;
}

REGISTER_COMM("usbl", RealUSBL)

}