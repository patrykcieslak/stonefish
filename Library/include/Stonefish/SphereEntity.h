//
//  SphereEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SphereEntity__
#define __Stonefish_SphereEntity__

#include "SolidEntity.h"

/*! Spherical solid entity. */
class SphereEntity : public SolidEntity
{
public:
    SphereEntity(std::string uniqueName, btScalar sphereRadius, Material* mat, Look l);
    ~SphereEntity();
    
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();

private:
    void BuildCollisionList();
    void BuildDisplayList();
    
    btScalar radius;
};

#endif
