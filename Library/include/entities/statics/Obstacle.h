//
//  Obstacle.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright(c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Obstacle__
#define __Stonefish_Obstacle__

#include "entities/StaticEntity.h"

namespace sf
{

//! A static obstacle loaded from a file or taking one of the simple geometrical shapes.
class Obstacle : public StaticEntity
{
public:
    Obstacle(std::string uniqueName, std::string modelFilename, btScalar scale, Material m, int lookId = -1, bool smoothNormals = false);
    Obstacle(std::string uniqueName, btScalar sphereRadius, Material m, int lookId = -1);
    Obstacle(std::string uniqueName, btVector3 boxDimensions, Material m, int lookId = -1);
    Obstacle(std::string uniqueName, btScalar cylinderRadius, btScalar cylinderHeight, Material m, int lookId = -1);
 
    StaticEntityType getStaticType();
};

}

#endif
