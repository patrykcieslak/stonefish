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
//  Copyright (c) 2014-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/Sample.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

Sample::Sample(unsigned short nDimensions, Scalar* values, bool invalid, uint64_t index)
{
    nDim = nDimensions > 0 ? nDimensions : 1;
    data = new Scalar[nDim];
    std::memcpy(data, values, sizeof(Scalar)*nDim);
    id = index;
    if(invalid)
        timestamp = Scalar(-1);
    else
        timestamp = SimulationApp::getApp()->getSimulationManager()->getSimulationTime();
}

Sample::Sample(const Sample& other, uint64_t index)
{
    timestamp = other.timestamp;
    nDim = other.nDim;
    data = new Scalar[nDim];
    std::memcpy(data, other.data, sizeof(Scalar)*nDim);
    id = index;
}

Sample::~Sample()
{
    delete [] data;
}

Scalar Sample::getTimestamp() const
{
    return timestamp;
}
    
unsigned short Sample::getNumOfDimensions() const
{
    return nDim;
}
    
Scalar* Sample::getDataPointer()
{
    return data;
}

Scalar Sample::getValue(unsigned short dimension) const
{
    if((dimension < nDim) && (data != NULL))
        return data[dimension];
    else
        return Scalar(0.);
}

std::vector<Scalar> Sample::getData() const
{
    return std::vector<Scalar>(data, data+nDim);
}

uint64_t Sample::getId() const
{
    return id;
}

}
