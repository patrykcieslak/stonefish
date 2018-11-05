//
//  Trigger.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/04/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Trigger__
#define __Stonefish_Trigger__

#include <entities/ForcefieldEntity.h>
#include <entities/SolidEntity.h>

class Trigger : public ForcefieldEntity
{
public:
	Trigger(std::string uniqueName, btScalar radius, const btTransform& worldTransform, int lookId = -1); //Spherical trigger
	Trigger(std::string uniqueName, btScalar radius, btScalar length, const btTransform& worldTransform, int lookId = -1); //Cylindrical trigger
	Trigger(std::string uniqueName, const btVector3& dimensions, const btTransform& worldTransform, int lookId = -1); //Box trigger
	void AddActiveSolid(SolidEntity* solid);
	void Activate(btCollisionObject* co);
	void Clear();
	std::vector<Renderable> Render();
	
	bool isActive();
	ForcefieldType getForcefieldType();
		
private:
	bool active;
	std::vector<SolidEntity*> solids;
	int objectId;
    int look;
};

#endif