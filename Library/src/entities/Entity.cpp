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
//  Entity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "entities/Entity.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Entity::Entity(std::string uniqueName)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    renderable = true;
}

Entity::~Entity(void)
{
    if(SimulationApp::getApp() != NULL)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

void Entity::setRenderable(bool render)
{
    renderable = render;
}

bool Entity::isRenderable()
{
    return renderable;
}

std::string Entity::getName()
{
    return name;
}
    
Vector3 Entity::findInertiaAxis(Matrix3 I, Scalar value)
{
    //Check if not I matrix already diagonal
    if(btFuzzyZero(I.getRow(0).getY()) && btFuzzyZero(I.getRow(0).getZ())
       && btFuzzyZero(I.getRow(1).getX()) && btFuzzyZero(I.getRow(1).getZ())
       && btFuzzyZero(I.getRow(2).getX()) && btFuzzyZero(I.getRow(2).getY()))
    {
        return Vector3(0,0,0);
    }
    
    //Diagonalize
    Matrix3 L;
    Vector3 candidates[3];
    Vector3 axis;
    
    //Characteristic matrix
    L = I - Matrix3::getIdentity().scaled(Vector3(value,value,value));
    
    //Candidates (orthogonal vectors)
    candidates[0] = (L.getRow(0).cross(L.getRow(1)));
    candidates[1] = (L.getRow(0).cross(L.getRow(2)));
    candidates[2] = (L.getRow(1).cross(L.getRow(2)));
    
    //Find best candidate
    if(candidates[0].length2() >= candidates[1].length2())
    {
        if(candidates[0].length2() >= candidates[2].length2())
            axis = candidates[0].normalized();
        else
            axis = candidates[2].normalized();
    }
    else
    {
        if(candidates[1].length2() >= candidates[2].length2())
            axis = candidates[1].normalized();
        else
            axis = candidates[2].normalized();
    }
    
    return axis;
}
    
}
