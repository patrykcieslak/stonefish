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
//  ForceTorque.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/ForceTorque.h"

#include "core/DeviceFactory.h"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"
#include "sensors/Sample.h"
#include "joints/Joint.h"

namespace sf
{

ForceTorque::ForceTorque(const std::string& uniqueName, SolidEntity* attachment, const Transform& origin, Scalar frequency, int historyLength)
    : JointSensor(uniqueName, frequency, historyLength)
{
    attach_ = attachment;
    o2s_ = origin;
    lastFrame_ = Transform::getIdentity();
    
    channels_.push_back(SensorChannel("Force X", QuantityType::FORCE));
    channels_.push_back(SensorChannel("Force Y", QuantityType::FORCE));
    channels_.push_back(SensorChannel("Force Z", QuantityType::FORCE));
    channels_.push_back(SensorChannel("Torque X", QuantityType::TORQUE));
    channels_.push_back(SensorChannel("Torque Y", QuantityType::TORQUE));
    channels_.push_back(SensorChannel("Torque Z", QuantityType::TORQUE));
}

ForceTorque::ForceTorque(const std::string& uniqueName, const Transform& origin, Scalar frequency, int historyLength) 
    : ForceTorque(uniqueName, nullptr, origin, frequency, historyLength)
{
}

Transform ForceTorque::getSensorFrame() const
{
    return lastFrame_;
}

void ForceTorque::InternalUpdate(Scalar dt)
{
    if(j_ != nullptr && attach_ != nullptr)
    {
        Vector3 force(j_->getFeedback(0), j_->getFeedback(1), j_->getFeedback(2));
        Vector3 torque(j_->getFeedback(3), j_->getFeedback(4), j_->getFeedback(5));
    
        if(j_->isMultibodyJoint())
        {  
            force /= dt;
            torque /= dt;
        }
    
        lastFrame_ = attach_->getOTransform() * o2s_;
        Matrix3 toSensor = lastFrame_.getBasis().inverse();
        force = toSensor * force;
        torque = toSensor * torque;

        AddSampleToHistory(std::make_unique<Sample>(
            std::vector<Scalar>({force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()})
        ));
    }
    else
    {   
        Vector3 force, torque;
        unsigned int childId = fe_->getJointFeedback(jId_, force, torque);
        lastFrame_ = fe_->getLink(childId).solid->getCG2OTransform() * o2s_;
        Matrix3 toSensor = lastFrame_.getBasis().inverse();
        force = toSensor * force;
        torque = toSensor * torque;
        lastFrame_ = fe_->getLink(childId).solid->getCGTransform() * lastFrame_; //From local to global
        
        AddSampleToHistory(std::make_unique<Sample>(
            std::vector<Scalar>({force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()})
        ));
    }
}

void ForceTorque::setRange(const Vector3& forceMax, const Vector3& torqueMax)
{
    channels_[0].rangeMin = -btClamped(forceMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[1].rangeMin = -btClamped(forceMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[2].rangeMin = -btClamped(forceMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[0].rangeMax = btClamped(forceMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[1].rangeMax = btClamped(forceMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[2].rangeMax = btClamped(forceMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    
    channels_[3].rangeMin = -btClamped(torqueMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[4].rangeMin = -btClamped(torqueMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[5].rangeMin = -btClamped(torqueMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[3].rangeMax = btClamped(torqueMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[4].rangeMax = btClamped(torqueMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[5].rangeMax = btClamped(torqueMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
}
    
void ForceTorque::setNoise(Scalar forceStdDev, Scalar torqueStdDev)
{
    channels_[0].setStdDev(btClamped(forceStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[1].setStdDev(btClamped(forceStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[2].setStdDev(btClamped(forceStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[3].setStdDev(btClamped(torqueStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[4].setStdDev(btClamped(torqueStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels_[5].setStdDev(btClamped(torqueStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}
    
std::vector<Renderable> ForceTorque::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SENSOR_CS;
        item.model = glMatrixFromTransform(lastFrame_);
        items.push_back(item);
    }    
    return items;
}

ScalarSensorType ForceTorque::getScalarSensorType() const
{
    return ScalarSensorType::FT;
}

// Statics

ConstructInfo ForceTorque::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // History
    node.optional = true;
    node.attributes.insert({"samples", {ConstructInfoValueType::INT, false}});
    info.nodes.insert({"history", node});

    // Origin
    node.attributes.clear();
    node.optional = false;
    node.attributes.insert({"T", {ConstructInfoValueType::TRANSFORM, false}});
    info.nodes.insert({"origin", node});

    // Range
    node.attributes.clear(); // Clear temporary
    node.optional = true;
    node.attributes.insert({"force", {ConstructInfoValueType::VECTOR3, true}});
    node.attributes.insert({"torque", {ConstructInfoValueType::VECTOR3, true}});
    info.nodes.insert({"range", node});

    // Noise
    node.attributes.clear(); // Clear temporary
    node.optional = true;
    node.attributes.insert({"force", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"torque", {ConstructInfoValueType::SCALAR, true}});
    info.nodes.insert({"noise", node});

    return info;
}

std::unique_ptr<ForceTorque> ForceTorque::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // History (optional)
    int history = -1;
    ConstructInfoValue& value = info.nodes.at("history").attributes.at("samples");
    if (value.valid)
        history = std::get<int>(value.value);
    
    // Origin (required)
    Transform origin = std::get<Transform>(info.nodes.at("origin").attributes.at("T").value);
    
    // Create sensor
    std::unique_ptr<ForceTorque> sensor = std::make_unique<ForceTorque>(uniqueName, origin, frequency, history);    

    // Range (optional)
    Vector3 forceMax = VMAX();
    Vector3 torqueMax = VMAX();
    value = info.nodes.at("range").attributes.at("force");
    if (value.valid)
        forceMax = std::get<Vector3>(value.value);

    value = info.nodes.at("range").attributes.at("torque");
    if (value.valid)
        torqueMax = std::get<Vector3>(value.value);

    sensor->setRange(forceMax, torqueMax);
    
    // Noise (optional)
    Scalar force (0.);
    Scalar torque (0.);
    
    value = info.nodes.at("noise").attributes.at("force");
    if (value.valid)
        force = std::get<Scalar>(value.value);
    
    value = info.nodes.at("noise").attributes.at("torque");
    if (value.valid)
        torque = std::get<Scalar>(value.value);

    sensor->setNoise(force, torque);

    return sensor;
}

REGISTER_SENSOR("force_torque", ForceTorque)

}
