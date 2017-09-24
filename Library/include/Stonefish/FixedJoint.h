//
//  FixedJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FixedJoint__
#define __Stonefish_FixedJoint__

#include "Joint.h"
#include "FeatherstoneEntity.h"

class FixedJoint : public Joint
{
public:
    FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB);
	FixedJoint(std::string uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA = -1, int linkIdB = -1);
    
    btVector3 Render();
    JointType getType();
    
    //Not applicable
    void ApplyDamping(){}
    bool SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance){ return true; }
};

#endif
