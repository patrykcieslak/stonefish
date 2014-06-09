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
    
    void ApplyForce(btScalar F);
    void ApplyTorque(btScalar T);
    void ApplyDamping();
    btVector3 Render();
    
    void setDamping(btScalar linearConstantFactor, btScalar linearViscousFactor, btScalar angularConstantFactor, btScalar angularViscousFactor);
    void setLimits(btScalar linearMin, btScalar linearMax, btScalar angularMin, btScalar angularMax);
    
    JointType getType();
    
private:
    btVector3 axisInA;
    btVector3 pivotInA;
    btScalar linSigDamping;
    btScalar linVelDamping;
    btScalar angSigDamping;
    btScalar angVelDamping;
};

#endif
