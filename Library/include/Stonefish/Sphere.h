//
//  Sphere.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sphere__
#define __Stonefish_Sphere__

#include "SolidEntity.h"

/*! Spherical solid entity. */
class Sphere : public SolidEntity
{
public:
    Sphere(std::string uniqueName, btScalar sphereRadius, Material m, int lookId = -1, btScalar thickness = btScalar(-1), bool isBuoyant = true);
    ~Sphere();
    
    SolidType getSolidType();
    btCollisionShape* BuildCollisionShape();

private:    
    btScalar radius;
};

#endif
