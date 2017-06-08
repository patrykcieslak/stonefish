//
//  Plane.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Plane__
#define __Stonefish_Plane__

#include "StaticEntity.h"
#include "MaterialManager.h"
#include "OpenGLContent.h"

class Plane : public StaticEntity
{
public:
    Plane(std::string uniqueName, btScalar size, Material* mat, const btTransform& worldTransform, int lookId = -1);
    ~Plane();
    
    void GetAABB(btVector3& min, btVector3& max);
    StaticEntityType getStaticType();
    
private:
    btScalar size;
};

#endif
