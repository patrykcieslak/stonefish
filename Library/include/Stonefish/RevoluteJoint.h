//
//  RevoluteJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RevoluteJoint__
#define __Stonefish_RevoluteJoint__

#include "Joint.h"

class RevoluteJoint : public Joint
{
public:
    RevoluteJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& pivot, const btVector3& axis, bool collideLinkedEntities = true);
    RevoluteJoint(std::string uniqueName, SolidEntity* solid, const btVector3& pivot, const btVector3& axis);
    ~RevoluteJoint();
    
    void ApplyTorque(btScalar T);
    void ApplyDamping();
   	btVector3 Render();
    
    void setDamping(btScalar constantFactor, btScalar viscousFactor);
    void setLimits(btScalar min, btScalar max);
    
    btScalar getAngle();
    btScalar getAngularVelocity();
    JointType getType();
    
private:
    btVector3 axisInA;
    btVector3 pivotInA;
    btScalar sigDamping;
    btScalar velDamping;
};

#endif
