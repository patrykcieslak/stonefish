//
//  PlaneEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PlaneEntity__
#define __Stonefish_PlaneEntity__

#include "StaticEntity.h"
#include "MaterialManager.h"
#include "OpenGLMaterial.h"

class PlaneEntity : public StaticEntity
{
public:
    PlaneEntity(std::string uniqueName, btScalar size, Material* mat, Look l, const btTransform& worldTransform);
    ~PlaneEntity();
    
    StaticEntityType getStaticType();
    
private:
    btScalar size;
};

#endif
