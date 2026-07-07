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
//  Copyright (c) 2017-2025 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/ForceTorque.h"

#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"
#include "sensors/Sample.h"
#include "joints/Joint.h"

namespace sf
{

ForceTorque::ForceTorque(std::string uniqueName, SolidEntity* attachment, const Transform& origin, Scalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
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

ForceTorque::ForceTorque(std::string uniqueName, const Transform& origin, Scalar frequency, int historyLength) :
    ForceTorque(uniqueName, NULL, origin, frequency, historyLength)
{
}

Transform ForceTorque::getSensorFrame() const
{
    return lastFrame_;
}

void ForceTorque::InternalUpdate(Scalar dt)
{
    if(j_ != NULL && attach_ != NULL)
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
	
        Sample s{std::vector<Scalar>({force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()})};
        AddSampleToHistory(s);
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
        
        Sample s{std::vector<Scalar>({force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()})};
        AddSampleToHistory(s);
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

}
