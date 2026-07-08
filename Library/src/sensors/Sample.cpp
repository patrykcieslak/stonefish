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
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/Sample.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

Sample::Sample(const std::vector<Scalar>& data, bool invalid, size_t index)
    : data_{data}, id_{index}
{
    if(invalid)
        timestamp_ = Scalar(-1);
    else
        timestamp_ = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
}

Scalar Sample::getTimestamp() const
{
    return timestamp_;
}
    
size_t Sample::getNumOfDimensions() const
{
    return data_.size();
}
    
Scalar* Sample::getDataPointer()
{
    return data_.data();
}

Scalar Sample::getValue(size_t dimension) const
{
    if(dimension < data_.size())
        return data_[dimension];
    else
        return Scalar(0);
}

std::vector<Scalar> Sample::getData() const
{
    return data_;
}

size_t Sample::getId() const
{
    return id_;
}

void Sample::setId(size_t id)
{
    id_ = id;
}

}
