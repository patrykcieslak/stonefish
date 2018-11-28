//
//  RevoluteJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RevoluteJoint__
#define __Stonefish_RevoluteJoint__

#include "joints/Joint.h"

namespace sf
{

class RevoluteJoint : public Joint
{
public:
    RevoluteJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, const Vector3& axis, bool collideLinkedEntities = true);
    RevoluteJoint(std::string uniqueName, SolidEntity* solid, const Vector3& pivot, const Vector3& axis);
    RevoluteJoint(std::string uniqueName, btRigidBody* bodyA, btRigidBody* bodyB, const Vector3& pivot, const Vector3& axis, bool collideLinkedEntities = true);
	
    void ApplyTorque(Scalar T);
    void ApplyDamping();
    bool SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance);
   	Vector3 Render();
    
    void setDamping(Scalar constantFactor, Scalar viscousFactor);
    void setLimits(Scalar min, Scalar max);
    void setIC(Scalar angle);
    
    Scalar getAngle();
    Scalar getAngularVelocity();
    JointType getType();
    
private:
    Vector3 axisInA;
    Vector3 pivotInA;
    Scalar sigDamping;
    Scalar velDamping;
    Scalar angleIC;
};
    
}

#endif
