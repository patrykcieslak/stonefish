//
//  FixedJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FixedJoint__
#define __Stonefish_FixedJoint__

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"

namespace sf
{

class FixedJoint : public Joint
{
public:
    FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB);
	FixedJoint(std::string uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB, const Vector3& pivot);
    
    Vector3 Render();
    JointType getType();
    
    //Not applicable
    void ApplyDamping(){}
    bool SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance){ return true; }
};

}
    
#endif
