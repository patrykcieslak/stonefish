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
//  Plane.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright(c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Plane.h"

#include "core/SimulationApp.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Plane::Plane(std::string uniqueName, Scalar planeSize, std::string material, std::string look, float uvScale) : StaticEntity(uniqueName, material, look)
{
    phyMesh = OpenGLContent::BuildPlane(planeSize/2.f, uvScale > 0.f ? uvScale : 1.f);
    btCollisionShape* shape = new btStaticPlaneShape(Vector3(0,0,-1), 0);
    shape->setMargin(COLLISION_MARGIN);
    BuildRigidBody(shape);
}

void Plane::getAABB(Vector3 &min, Vector3 &max)
{
    //Plane shouldn't affect shadow calculation
    min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
}

StaticEntityType Plane::getStaticType()
{
    return StaticEntityType::PLANE;
}

}
