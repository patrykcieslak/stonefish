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
//  Sample.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014-2025 Patryk Cieslak. All rights reserved.
//

#include "sensors/Sample.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

Sample::Sample(const std::vector<Scalar>& data, bool invalid, uint64_t index)
    : data{data}, id{index}
{
    if(invalid)
        timestamp = Scalar(-1);
    else
        timestamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
}

Sample::Sample(const Sample& other, uint64_t index)
{
    timestamp = other.timestamp;
    data = other.data;
    id = index;
}

Scalar Sample::getTimestamp() const
{
    return timestamp;
}
    
size_t Sample::getNumOfDimensions() const
{
    return data.size();
}
    
Scalar* Sample::getDataPointer()
{
    return data.data();
}

Scalar Sample::getValue(size_t dimension) const
{
    if(dimension < data.size())
        return data[dimension];
    else
        return Scalar(0);
}

std::vector<Scalar> Sample::getData() const
{
    return data;
}

uint64_t Sample::getId() const
{
    return id;
}

}
