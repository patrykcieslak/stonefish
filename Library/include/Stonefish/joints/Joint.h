//
//  Joint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Joint__
#define __Stonefish_Joint__

#include <common.h>
#include <entities/SolidEntity.h>
#include <BulletDynamics/Featherstone/btMultiBodyConstraint.h>

#define CONSTRAINT_ERP 0.2
#define CONSTRAINT_CFM 0.0
#define CONSTRAINT_STOP_ERP 1.0
#define CONSTRAINT_STOP_CFM 0.0

typedef enum {JOINT_FIXED, JOINT_REVOLUTE, JOINT_SPHERICAL, JOINT_PRISMATIC, JOINT_CYLINDRICAL, JOINT_GEAR, JOINT_BELT, JOINT_SCREW} JointType;

//abstract class
class Joint
{
public:
    Joint(std::string uniqueName, bool collideLinkedEntities = true);
    virtual ~Joint();
    
    virtual void ApplyDamping() = 0;
	virtual bool SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance) = 0;
    virtual btVector3 Render() = 0;
    virtual JointType getType() = 0;
    
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    
    void setRenderable(bool render);
    btTypedConstraint* getConstraint();
    
    //In the world frame
    btScalar getFeedback(unsigned int dof);
    
    std::string getName();
    
    bool isRenderable();
    bool isMultibodyJoint();
    
protected:
    void setConstraint(btTypedConstraint* c);
	void setConstraint(btMultiBodyConstraint* c);
    
private:
    std::string name;
    bool renderable;
    bool collisionEnabled;
    btTypedConstraint* constraint;
	btMultiBodyConstraint* mbConstraint;
};



#endif
