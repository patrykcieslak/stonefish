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
//  Copyright (c) 2017-2021 Patryk Cieslak. All rights reserved.
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
    attach = attachment;
    o2s = origin;
    lastFrame = Transform::getIdentity();
    
    channels.push_back(SensorChannel("Force X", QuantityType::FORCE));
    channels.push_back(SensorChannel("Force Y", QuantityType::FORCE));
    channels.push_back(SensorChannel("Force Z", QuantityType::FORCE));
    channels.push_back(SensorChannel("Torque X", QuantityType::TORQUE));
    channels.push_back(SensorChannel("Torque Y", QuantityType::TORQUE));
    channels.push_back(SensorChannel("Torque Z", QuantityType::TORQUE));
}

ForceTorque::ForceTorque(std::string uniqueName, const Transform& origin, Scalar frequency, int historyLength) :
    ForceTorque(uniqueName, NULL, origin, frequency, historyLength)
{
}

Transform ForceTorque::getSensorFrame() const
{
    return lastFrame;
}

void ForceTorque::InternalUpdate(Scalar dt)
{
    if(j != NULL && attach != NULL)
    {
        Vector3 force(j->getFeedback(0), j->getFeedback(1), j->getFeedback(2));
        Vector3 torque(j->getFeedback(3), j->getFeedback(4), j->getFeedback(5));
    
        if(j->isMultibodyJoint())
        {  
            force /= dt;
            torque /= dt;
        }
    
        lastFrame = attach->getOTransform() * o2s;
        Matrix3 toSensor = lastFrame.getBasis().inverse();
        force = toSensor * force;
        torque = toSensor * torque;
	
        Scalar values[6] = {force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()};
        Sample s(6, values);
        AddSampleToHistory(s);
    }
    else
    {   
        Vector3 force, torque;
        unsigned int childId = fe->getJointFeedback(jId, force, torque);
        lastFrame = fe->getLink(childId).solid->getCG2OTransform() * o2s;
        Matrix3 toSensor = lastFrame.getBasis().inverse();
        force = toSensor * force;
        torque = toSensor * torque;
        lastFrame = fe->getLink(childId).solid->getCGTransform() * lastFrame; //From local to global
        
        Scalar values[6] = {force.getX(), force.getY(), force.getZ(), torque.getX(), torque.getY(), torque.getZ()};
        Sample s(6, values);
        AddSampleToHistory(s);
    }
}

void ForceTorque::setRange(const Vector3& forceMax, const Vector3& torqueMax)
{
    channels[0].rangeMin = -btClamped(forceMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[1].rangeMin = -btClamped(forceMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[2].rangeMin = -btClamped(forceMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[0].rangeMax = btClamped(forceMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[1].rangeMax = btClamped(forceMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[2].rangeMax = btClamped(forceMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    
    channels[3].rangeMin = -btClamped(torqueMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[4].rangeMin = -btClamped(torqueMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[5].rangeMin = -btClamped(torqueMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[3].rangeMax = btClamped(torqueMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[4].rangeMax = btClamped(torqueMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[5].rangeMax = btClamped(torqueMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
}
    
void ForceTorque::setNoise(Scalar forceStdDev, Scalar torqueStdDev)
{
    channels[0].setStdDev(btClamped(forceStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[1].setStdDev(btClamped(forceStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[2].setStdDev(btClamped(forceStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[3].setStdDev(btClamped(torqueStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[4].setStdDev(btClamped(torqueStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    channels[5].setStdDev(btClamped(torqueStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}
    
std::vector<Renderable> ForceTorque::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SENSOR_CS;
        item.model = glMatrixFromTransform(lastFrame);
        items.push_back(item);
    }    
    return items;
}

ScalarSensorType ForceTorque::getScalarSensorType()
{
    return ScalarSensorType::FT;
}

}
