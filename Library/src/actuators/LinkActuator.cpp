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
//  LinkActuator.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 23/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "actuators/LinkActuator.h"

#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

LinkActuator::LinkActuator(std::string uniqueName) : Actuator(uniqueName)
{
    attach = NULL;
    o2a = Transform::getIdentity();
}

ActuatorType LinkActuator::getType()
{
    return ActuatorType::ACTUATOR_LINK;
}
    
Transform LinkActuator::getActuatorFrame()
{
    if(attach != NULL)
        return attach->getOTransform() * o2a;
    else
        return o2a;
}

void LinkActuator::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId, const Transform& origin)
{
    if(multibody != NULL && linkId < multibody->getNumOfLinks())
    {
        o2a = origin;
        attach = multibody->getLink(linkId).solid;
    }
}

void LinkActuator::AttachToSolid(SolidEntity* solid, const Transform& origin)
{
    if(solid != NULL)
    {
        o2a = origin;
        attach = solid;
    }
}
    
}
