//
//  Plane.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright(c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Plane.h"

#include "core/SimulationApp.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Plane::Plane(std::string uniqueName, Scalar planeSize, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    phyMesh = OpenGLContent::BuildPlane(planeSize/2.f);
    btCollisionShape* shape = new btStaticPlaneShape(Vector3(0,0,-1), 0);
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
    return STATIC_PLANE;
}

}
