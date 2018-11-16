//
//  TwoFingerGripper.h
//  Stonefish
//
//  Created by Patryk Cieslak on 06/07/18.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_TwoFingerGripper__
#define __Stonefish_TwoFingerGripper__

#include "entities/systems/Gripper.h"

//! A two finger gripper.
/*! 
 * This class implements a model of a two finger gripper equipped with a force/torque sensor. 
 */
class TwoFingerGripper : public Gripper
{
public:
    TwoFingerGripper(std::string uniqueName, Manipulator* m, SolidEntity* hand, SolidEntity* finger1, SolidEntity* finger2, const btVector3& pivotA, const btVector3& pivotB, const btVector3& axis, btScalar openAngle, btScalar closingTorque);
    
    void SetState(btScalar openFraction);
    
private:
    btScalar open;
};

#endif
