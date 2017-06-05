//
//  BoxEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_BoxEntity__
#define __Stonefish_BoxEntity__

#include "SolidEntity.h"

class BoxEntity : public SolidEntity
{
public:
    BoxEntity(std::string uniqueName, const btVector3& dimensions, Material* mat, Look l);
    ~BoxEntity();
    
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();

private:
    void BuildCollisionList();
    
    btVector3 halfExtents;
};

#endif
