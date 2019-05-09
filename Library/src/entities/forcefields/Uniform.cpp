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
//  Uniform.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/05/19.
//  Copyright(c) 2019 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Uniform.h"

namespace sf
{

Uniform::Uniform(const Vector3& velocity)
{
    v = velocity;
}

Vector3 Uniform::GetVelocityAtPoint(const Vector3& p)
{    
    return v;
}

std::vector<Renderable> Uniform::Render()
{
    std::vector<Renderable> items(0);
    return items;
}

}

