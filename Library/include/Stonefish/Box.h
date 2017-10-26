//
//  Box.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Box__
#define __Stonefish_Box__

#include "SolidEntity.h"

class Box : public SolidEntity
{
public:
    Box(std::string uniqueName, const btVector3& dimensions, Material m, int lookId = -1, btScalar thickness = btScalar(-1), bool isBuoyant = true);
    ~Box();
    
    SolidType getSolidType();
    btCollisionShape* BuildCollisionShape();

private:
    btVector3 halfExtents;
};

#endif
