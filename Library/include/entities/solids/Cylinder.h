//
//  Cylinder.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Cylinder__
#define __Stonefish_Cylinder__

#include "entities/SolidEntity.h"

namespace sf
{

class Cylinder : public SolidEntity
{
public:
    Cylinder(std::string uniqueName, Scalar cylinderRadius, Scalar cylinderHeight, const Transform& originTrans, Material m, int lookId = -1, Scalar thickness = Scalar(-1), bool isBuoyant = true);
    ~Cylinder();
    
    SolidType getSolidType();
    btCollisionShape* BuildCollisionShape();

private:
    Scalar radius;
    Scalar halfHeight;
};

}

#endif
