//
//  Trigger.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/04/18.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Trigger__
#define __Stonefish_Trigger__

#include "entities/ForcefieldEntity.h"
#include "entities/SolidEntity.h"

namespace sf
{

class Trigger : public ForcefieldEntity
{
public:
	Trigger(std::string uniqueName, Scalar radius, const Transform& worldTransform, int lookId = -1); //Spherical trigger
	Trigger(std::string uniqueName, Scalar radius, Scalar length, const Transform& worldTransform, int lookId = -1); //Cylindrical trigger
	Trigger(std::string uniqueName, const Vector3& dimensions, const Transform& worldTransform, int lookId = -1); //Box trigger
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

}

#endif
