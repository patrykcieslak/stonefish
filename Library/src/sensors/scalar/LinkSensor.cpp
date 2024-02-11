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
//  LinkSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/11/2018.
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/LinkSensor.h"

#include "entities/MovingEntity.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

LinkSensor::LinkSensor(std::string uniqueName, Scalar frequency, int historyLength) : ScalarSensor(uniqueName, frequency, historyLength)
{
    attach = nullptr;
    o2s = Transform::getIdentity();
}

LinkSensor::~LinkSensor()
{
}

void LinkSensor::setRelativeSensorFrame(const Transform& origin)
{
    o2s = origin;
}

Transform LinkSensor::getSensorFrame() const
{
    if(attach != nullptr)
        return attach->getOTransform() * o2s;
    else
        return o2s;
}

SensorType LinkSensor::getType()
{
    return SensorType::LINK;
}

std::string LinkSensor::getLinkName()
{
    if(attach != nullptr)
        return attach->getName();
    else
        return std::string("");
}

void LinkSensor::AttachToSolid(MovingEntity* solid, const Transform& origin)
{
    if(solid != nullptr)
    {
        o2s = origin;
        attach = solid;
    }
}

std::vector<Renderable> LinkSensor::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SENSOR_CS;
        item.model = glMatrixFromTransform(getSensorFrame());
        items.push_back(item);
    }
    return items;
}

}
