//
//  Obstacle.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Obstacle__
#define __Stonefish_Obstacle__

#include "StaticEntity.h"

class Obstacle : public StaticEntity
{
public:
    Obstacle(std::string uniqueName, const char* modelFilename, btScalar scale, Material* mat, const btTransform& worldTransform, int lookId = -1, bool smoothNormals = false);
    ~Obstacle();
    
    StaticEntityType getStaticType();
    
private:
    Mesh *mesh;
    btTriangleIndexVertexArray* triangleArray;
};

#endif
