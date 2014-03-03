//
//  MeshEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MeshEntity__
#define __Stonefish_MeshEntity__

#include "SolidEntity.h"

class MeshEntity : public SolidEntity
{
public:
    MeshEntity(std::string uniqueName, const char* modelFilename, btScalar scale, bool isStatic, Material* mat, Look l, bool smoothNormals);
	~MeshEntity();
    
    void SetLook(Look newLook);
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();
    void CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                btTransform* worldTransform = NULL, const btVector3& velocity = btVector3(0,0,0),
                                const btVector3& angularVelocity = btVector3(0,0,0));
    
private:
    void BuildCollisionList();
    btTriangleIndexVertexArray* triangleArray;
    btVector3 aabb[2];
};


#endif
