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
    ~RevoluteJoint();
    
   	JointType getType();
    void Render();
    
    btScalar getAngle();
    btScalar getAngularVelocity();
    void applyTorque(btScalar T);
    void setTargetVelocity(btScalar v, btScalar maxT);
    btScalar getTargetVelocity();
    btVector3 getAxis();
    
private:
    btVector3 axisInA;
};

#endif
