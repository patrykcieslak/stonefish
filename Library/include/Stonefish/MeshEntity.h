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
    MeshEntity(std::string uniqueName, const char* modelFilename, btScalar scale, Material* mat, Look l, bool smoothNormals = true);
	~MeshEntity();
    
    void SetLook(Look newLook);
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();
    
    void ComputeFluidForces(const FluidEntity* fluid, const btTransform& cogTransform, const btTransform& geometryTransform, const btVector3& linearV, const btVector3& angularV, const btVector3& linearA, const btVector3& angularA, btVector3& Fb, btVector3& Tb, btVector3& Fd, btVector3& Td, btVector3& Fa, btVector3& Ta, bool damping = true, bool addedMass = true);
    
private:
    void BuildCollisionList();
    btTriangleIndexVertexArray* triangleArray;
    btVector3 aabb[2];
};


#endif
