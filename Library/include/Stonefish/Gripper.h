//
//  Gripper.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Gripper__
#define __Stonefish_Gripper__

#include "SystemEntity.h"
#include "Manipulator.h"
#include "FixedJoint.h"
#include "ForceTorque.h"

//! A manipulator gripper
/*! 
 * This abstract class implements a model of a general gripper equipped with a force/torque sensor. 
 */
class Gripper : public SystemEntity
{
public:
    Gripper(std::string uniqueName, Manipulator* m);
    virtual ~Gripper();
    
    virtual void SetState(btScalar openFraction) = 0;
    btScalar GetState();
    void Open();
    void Close();
    
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
    void UpdateAcceleration(btScalar dt);
    void UpdateSensors(btScalar dt);
    void ApplyGravity(const btVector3& g);
	void ApplyDamping();
    
    void GetAABB(btVector3& min, btVector3& max);
    std::vector<Renderable> Render();
    btTransform getTransform() const;
    SystemType getSystemType();
    ForceTorque* getFT();
    
    
protected:
    FeatherstoneEntity* mechanism;
    btScalar openFrac;
    
private:
    FixedJoint* fix;
    Manipulator* manipulator;
    ForceTorque* ft;
};

#endif
