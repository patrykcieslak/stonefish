//
//  BeltJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_BeltJoint__
#define __Stonefish_BeltJoint__

#include "joints/Joint.h"

namespace sf
{

class BeltJoint : public Joint
{
public:
    BeltJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& axisA, const btVector3& axisB, btScalar ratio);
    
    btVector3 Render();
    
    JointType getType();
    btScalar getRatio();
    
    //Not applicable
    void ApplyDamping(){}
    bool SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance){ return true; }
    
private:
    btScalar gearRatio;
};
    
}

#endif
