//
//  SphericalJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SphericalJoint__
#define __Stonefish_SphericalJoint__

#include "Joint.h"

class SphericalJoint : public Joint
{
public:
    SphericalJoint(btRigidBody* bodyA, btRigidBody* bodyB, const btVector3& pivot);
    ~SphericalJoint();
    
    JointType getType();
    void Render();
    
private:
};

#endif