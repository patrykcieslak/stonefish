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
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/JointSensor.h"

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"

namespace sf
{

JointSensor::JointSensor(std::string uniqueName, Scalar frequency, int historyLength) : ScalarSensor(uniqueName, frequency, historyLength)
{
    fe = NULL;
    jId = 0;
    j = NULL;
}

SensorType JointSensor::getType()
{
    return SensorType::JOINT;
}

Transform JointSensor::getSensorFrame() const
{
    return I4();
}

std::string JointSensor::getJointName()
{
    if(j != NULL)
        return j->getName();
    else if(fe != NULL)
        return fe->getJointName(jId);
    else
        return std::string("");
}

void JointSensor::AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId)
{
    if(multibody != NULL && jointId < multibody->getNumOfJoints())
    {
        fe = multibody;
        jId = jointId;
    }
}

void JointSensor::AttachToJoint(Joint* joint)
{
    if(joint != NULL)
        j = joint;
}

}
