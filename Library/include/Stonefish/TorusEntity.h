//
//  TorusEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 3/5/14.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_TorusEntity__
#define __Stonefish_TorusEntity__

#include "SolidEntity.h"
#define MESH_RESOLUTION 24

class TorusEntity : public SolidEntity
{
public:
    TorusEntity(std::string uniqueName, btScalar torusMajorRadius, btScalar torusMinorRadius, Material* mat, Look l);
    ~TorusEntity();
    
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();
    
private:
    void BuildCollisionList();
    void BuildDisplayList();
    
    btScalar minorRadius;
    btScalar majorRadius;
};

#endif
