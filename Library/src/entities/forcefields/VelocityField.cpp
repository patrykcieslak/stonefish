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
//  VelocityField.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/VelocityField.h"

namespace sf
{

VelocityField::VelocityField() : enabled(true)
{
}

VelocityField::~VelocityField()
{
}

void VelocityField::setEnabled(bool en)
{
    enabled = en;
}

bool VelocityField::isEnabled() const
{
    return enabled;
}

}
