//
//  ObstacleEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ObstacleEntity__
#define __Stonefish_ObstacleEntity__

#include "StaticEntity.h"
#include "GeometryUtil.h"

class ObstacleEntity : public StaticEntity
{
public:
    ObstacleEntity(std::string uniqueName, const char* modelFilename, btScalar scale, Material* mat, Look l, const btTransform& worldTransform, bool smoothNormals = false);
    ~ObstacleEntity();
    
    StaticEntityType getStaticType();
    
private:
    TriangleMesh *mesh;
    btTriangleIndexVertexArray* triangleArray;
};

#endif
