//
//  PrismaticJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 27/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PrismaticJoint__
#define __Stonefish_PrismaticJoint__

#include "joints/Joint.h"

namespace sf
{

class PrismaticJoint : public Joint
{
public:
    PrismaticJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& axis, bool collideLinkedEntities = true);
    
    void ApplyForce(Scalar F);
    void ApplyDamping();
    bool SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance);
    Vector3 Render();
    
    void setDamping(Scalar constantFactor, Scalar viscousFactor);
    void setLimits(Scalar min, Scalar max);
    void setIC(Scalar displacement);
    
    JointType getType();
    
private:
    Vector3 axisInA;
    Scalar sigDamping;
    Scalar velDamping;
    Scalar displacementIC;
};
    
}

#endif
