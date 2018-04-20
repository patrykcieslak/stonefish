//
//  ScrewJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/04/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ScrewJoint__
#define __Stonefish_ScrewJoint__

#include "Joint.h"
#include "FeatherstoneEntity.h"

class ScrewJoint : public Joint
{
public:    
    ScrewJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB);
	
    void ApplyDamping();
    bool SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance);
    btVector3 Render();
    JointType getType();
};

#endif