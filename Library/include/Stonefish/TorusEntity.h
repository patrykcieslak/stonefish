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
    TorusEntity(std::string uniqueName, btScalar torusMajorRadius, btScalar torusMinorRadius, bool isStatic, Material* mat, Look l);
    ~TorusEntity();
    
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();
    void CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                btTransform* worldTransform = NULL, const btVector3& velocity = btVector3(0,0,0),
                                const btVector3& angularVelocity = btVector3(0,0,0));
    
private:
    void BuildCollisionList();
    void BuildDisplayList();
    
    btScalar minorRadius;
    btScalar majorRadius;
};

#endif
