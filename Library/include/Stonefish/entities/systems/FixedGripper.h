//
//  FixedGripper.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FixedGripper__
#define __Stonefish_FixedGripper__

#include "Gripper.h"

//! A fixed manipulator gripper
/*! 
 * This class implements a model of a fixed gripper equipped with a force/torque sensor. 
 */
class FixedGripper : public Gripper
{
public:
    FixedGripper(std::string uniqueName, Manipulator* m, SolidEntity* hand);
    
    void SetState(btScalar openFraction);
};

#endif