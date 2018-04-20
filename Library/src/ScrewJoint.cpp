//
//  ScrewJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/04/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "ScrewJoint.h"

ScrewJoint::ScrewJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB) : Joint(uniqueName, false)
{
    
}
	
void ScrewJoint::ApplyDamping()
{
    
    
}

bool ScrewJoint::SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance)
{
    
}

btVector3 ScrewJoint::Render()
{
    return btVector3();
}

JointType ScrewJoint::getType()
{
    return JOINT_SCREW;
}