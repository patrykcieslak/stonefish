//
//  CylinderEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_CylinderEntity__
#define __Stonefish_CylinderEntity__

#include "SolidEntity.h"
#define MESH_RESOLUTION 24

class CylinderEntity : public SolidEntity
{
public:
    CylinderEntity(std::string uniqueName, btScalar cylinderRadius, btScalar cylinderHeight, Material* mat, Look l);
    ~CylinderEntity();
    
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();

private:
    void BuildCollisionList();
    
    btScalar radius;
    btScalar halfHeight;
};

#endif
