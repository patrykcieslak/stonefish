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
    
    void ApplyForce(btScalar F);
    void ApplyDamping();
    bool SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance);
    btVector3 Render();
    
    void setDamping(btScalar constantFactor, btScalar viscousFactor);
    void setLimits(btScalar min, btScalar max);
    void setIC(btScalar displacement);
    
    JointType getType();
    
private:
    btVector3 axisInA;
    btScalar sigDamping;
    btScalar velDamping;
    btScalar displacementIC;
};

#endif
