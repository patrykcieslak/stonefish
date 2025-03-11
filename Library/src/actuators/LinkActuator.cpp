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
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#include "actuators/LinkActuator.h"

#include "entities/SolidEntity.h"

namespace sf
{

LinkActuator::LinkActuator(std::string uniqueName) : Actuator(uniqueName)
{
    attach = nullptr;
    o2a = I4();
}

void LinkActuator::setRelativeActuatorFrame(const Transform& origin)
{
    o2a = origin;
}

Transform LinkActuator::getActuatorFrame() const
{
    if(attach != nullptr)
        return attach->getOTransform() * o2a;
    else
        return o2a;
}

void LinkActuator::AttachToSolid(SolidEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        o2a = origin;
        attach = body;
    }
}

std::vector<Renderable> LinkActuator::Render()
{
    std::vector<Renderable> items(0);

    Renderable item;
    item.type = RenderableType::SENSOR_CS;
    item.model = glMatrixFromTransform(getActuatorFrame());
    items.push_back(item);

    return items;
}
    
}
