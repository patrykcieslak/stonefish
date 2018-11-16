//
//  SphericalJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SphericalJoint__
#define __Stonefish_SphericalJoint__

#include "joints/Joint.h"

class SphericalJoint : public Joint
{
public:
    SphericalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& pivot, bool collideLinkedEntities = true);
    
    void ApplyTorque(btVector3 T);
    void ApplyDamping();
    bool SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance);
    btVector3 Render();
    
    void setDamping(btVector3 constantFactor, btVector3 viscousFactor);
    void setIC(btVector3 angles);
    
    JointType getType();
    
private:
    btVector3 sigDamping;
    btVector3 velDamping;
    btVector3 angleIC;
};

#endif
