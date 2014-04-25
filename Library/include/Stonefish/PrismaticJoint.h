//
//  PrismaticJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 27/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PrismaticJoint__
#define __Stonefish_PrismaticJoint__

#include "Joint.h"

class PrismaticJoint : public Joint
{
public:
    PrismaticJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& axis, bool collideLinkedEntities = true);
    ~PrismaticJoint();
    
    JointType getType();
    btVector3 Render();
    
private:
    btVector3 axisInA;
};

#endif
