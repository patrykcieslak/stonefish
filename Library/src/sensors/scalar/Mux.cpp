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
//  Mux.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Mux.h"

#include "sensors/ScalarSensor.h"
#include "sensors/Sample.h"

namespace sf
{

bool Mux::AddComponent(ScalarSensor* s, size_t channel)
{
    if(s == nullptr)
        return false;
    
    if(channel >= s->getNumOfChannels())
        return false;
    
    MuxComponent cmp;
    cmp.sensor = s;
    cmp.channel = channel;
    components_.push_back(cmp);

    return true;
}

MuxComponent* Mux::getComponent(size_t index)
{
    if(index < components_.size())
        return &components_[index];
    
    return nullptr;
}

std::vector<Scalar> Mux::getLastSample()
{
    std::vector<Scalar> sample(components_.size());
    for(size_t i = 0; i < components_.size(); ++i)
        sample[i] = components_[i].sensor->getLastSample().getValue(components_[i].channel);

    return sample;
}

size_t Mux::getNumOfComponents() const
{
    return components_.size();
}
    
}
