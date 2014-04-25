//
//  CylindricalJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_CylindricalJoint__
#define __Stonefish_CylindricalJoint__

#include "Joint.h"

class CylindricalJoint : public Joint
{
public:
    CylindricalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& pivot, const btVector3& axis, bool collideLinkedEntities = true);
    ~CylindricalJoint();
    
    JointType getType();
    btVector3 Render();
    
private:
    btVector3 axisInA;
    btVector3 pivotInA;
};

#endif
