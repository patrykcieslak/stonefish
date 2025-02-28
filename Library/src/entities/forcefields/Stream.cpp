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
//  Stream.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Stream.h"

namespace sf
{

Stream::Stream(const std::vector<Vector3>& streamline, const std::vector<Scalar>& radius, Scalar inputVelocity, Scalar exponent)
{
    c = streamline;
    r = radius;
    vin = inputVelocity;
    gamma = exponent;
}

VelocityFieldType Stream::getType() const
{
    return VelocityFieldType::STREAM;
}

Vector3 Stream::GetVelocityAtPoint(const Vector3& p) const
{
    return Vector3(0,0,0);
}

std::vector<Renderable> Stream::Render(VelocityFieldUBO& ubo)
{
    ubo.posR = glm::vec4(0.f);
    ubo.dirV = glm::vec4(0.f);
    ubo.params = glm::vec3(0.f);
    ubo.type = 0;
    return std::vector<Renderable>(0);
}
    
}
