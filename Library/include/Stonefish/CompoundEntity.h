//
//  CompoundEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/31/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__CompoundEntity__
#define __Stonefish__CompoundEntity__

#include "SolidEntity.h"
#include <vector>

class CompoundEntity : public SolidEntity
{
public:
    CompoundEntity(std::string uniqueName, bool isStatic);
    ~CompoundEntity();
    
    SolidEntityType getSolidType();
    btCollisionShape* BuildCollisionShape();
    void CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                btTransform* worldTransform = NULL, const btVector3& velocity = btVector3(0,0,0),
                                const btVector3& angularVelocity = btVector3(0,0,0));
    
    void AddSolid(SolidEntity* solid, const btTransform& location);
    void RemoveSolid(unsigned int index);
    SolidEntity* GetSolid(unsigned int index);
    unsigned int SolidsCount();
    
private:
    void BuildRigidBody();
    void BuildCollisionList();
    void BuildDisplayList();
    
    std::vector<SolidEntity*> solids;
    std::vector<btTransform> solidLocations;
};

#endif
