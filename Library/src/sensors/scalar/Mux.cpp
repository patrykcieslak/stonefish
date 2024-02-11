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
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Mux.h"

#include "sensors/ScalarSensor.h"
#include "sensors/Sample.h"

namespace sf
{

Mux::Mux()
{
}

Mux::~Mux()
{
    components.clear();
}

bool Mux::AddComponent(ScalarSensor* s, unsigned short channel)
{
    if(s == NULL)
        return false;
    
    if(channel >= s->getNumOfChannels())
        return false;
    
    MuxComponent cmp;
    cmp.sensor = s;
    cmp.channel = channel;
    components.push_back(cmp);
    return true;
}

MuxComponent* Mux::getComponent(unsigned int index)
{
    if(index < components.size())
        return &components[index];
    
    return NULL;
}

Scalar* Mux::getLastSample()
{
    Scalar* sample = new Scalar[components.size()];
    for(unsigned int i = 0; i < components.size(); ++i)
        sample[i] = components[i].sensor->getLastSample().getValue(components[i].channel);
    return sample;
}

unsigned int Mux::getNumOfComponents()
{
    return (unsigned int)components.size();
}
    
}
