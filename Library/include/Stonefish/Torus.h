//
//  Torus.h
//  Stonefish
//
//  Created by Patryk Cieslak on 3/5/14.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Torus__
#define __Stonefish_Torus__

#include "SolidEntity.h"

class Torus : public SolidEntity
{
public:
    Torus(std::string uniqueName, btScalar torusMajorRadius, btScalar torusMinorRadius, Material m, int lookId = -1);
    ~Torus();
    
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();
    
private:	
    btScalar minorRadius;
    btScalar majorRadius;
};

#endif
