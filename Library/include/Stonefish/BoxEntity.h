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
    BoxEntity(std::string uniqueName, const btVector3& dimensions, Material* mat, Look l, bool isStatic = false);
    ~BoxEntity();
    
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();
    void CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                btTransform* worldTransform = NULL, const btVector3& velocity = btVector3(0,0,0),
                                const btVector3& angularVelocity = btVector3(0,0,0));
    
private:
    void BuildCollisionList();
    
    btVector3 halfExtents;
};

#endif