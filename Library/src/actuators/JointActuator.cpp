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
//  JointActuator.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 23/11/2018.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/JointActuator.h"

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"

namespace sf
{

JointActuator::JointActuator(const std::string& uniqueName) : Actuator(uniqueName)
{
    fe_ = nullptr;
    jId_ = 0;
    j_ = nullptr;
}

const std::string& JointActuator::getJointName() const
{
    if(fe_ != nullptr)
        return fe_->getJointName(jId_);
    else if(j_ != nullptr)
        return j_->getName();
    else
        return std::string("");
}

void JointActuator::AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId)
{
    if(multibody != nullptr && jointId < multibody->getNumOfJoints())
    {
        fe_ = multibody;
        jId_ = jointId;
    }
}

void JointActuator::AttachToJoint(Joint* joint)
{
    if(joint != nullptr)
        j_ = joint;
}
    
}
