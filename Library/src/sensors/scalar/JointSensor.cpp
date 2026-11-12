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
//  JointSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/11/2018.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/JointSensor.h"

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"

namespace sf
{

JointSensor::JointSensor(std::string_view uniqueName, Scalar frequency, int historyLength) : ScalarSensor(uniqueName, frequency, historyLength)
{
    fe_ = nullptr;
    jId_ = 0;
    j_ = nullptr;
}

SensorType JointSensor::GetType() const
{
    return SensorType::JOINT;
}

Transform JointSensor::GetSensorFrame() const
{
    return I4();
}

void JointSensor::GetSensorVelocity(Vector3& linear, Vector3& angular) const
{
    linear = V0();
    angular = V0();
}

std::string JointSensor::GetJointName() const
{
    if(j_ != nullptr)
        return j_->GetName();
    else if(fe_ != nullptr)
        return fe_->GetJointName(jId_);
    else
        return std::string("");
}

void JointSensor::AttachToJoint(FeatherstoneEntity* multibody, size_t jointId)
{
    if(multibody != nullptr && jointId < multibody->GetNumOfJoints())
    {
        fe_ = multibody;
        jId_ = jointId;
    }
}

void JointSensor::AttachToJoint(Joint* joint)
{
    if(joint != nullptr)
        j_ = joint;
}

}
