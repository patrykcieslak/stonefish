//
//  Joint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "joints/Joint.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"

namespace sf
{

Joint::Joint(std::string uniqueName, bool collideLinkedEntities)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    renderable = false;
    collisionEnabled = collideLinkedEntities;
	mbConstraint = NULL;
	constraint = NULL;
}

Joint::~Joint(void)
{
	SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

void Joint::setRenderable(bool render)
{
    renderable = render;
}

bool Joint::isRenderable()
{
    return renderable;
}

bool Joint::isMultibodyJoint()
{
    return (constraint == NULL) && (mbConstraint != NULL);
}

btTypedConstraint* Joint::getConstraint()
{
    return constraint;
}

std::string Joint::getName()
{
    return name;
}

void Joint::setConstraint(btTypedConstraint *c)
{
    constraint = c;
}

void Joint::setConstraint(btMultiBodyConstraint *c)
{
    mbConstraint = c;
}

Scalar Joint::getFeedback(unsigned int dof)
{
    if(dof > 5)
        return Scalar(0);
    
    if(constraint != NULL)
    {
        btJointFeedback* fb = constraint->getJointFeedback();
        if(dof < 3)
            return fb->m_appliedForceBodyA[dof];
        else
            return fb->m_appliedTorqueBodyA[dof-3];
    }
    else if(mbConstraint != NULL)
    {
        return mbConstraint->getAppliedImpulse(dof);
    }
    else
        return Scalar(0);
}

void Joint::AddToSimulation(SimulationManager* sm)
{
	if(constraint != NULL)
	{
		//Force feedback
		btJointFeedback* fb = new btJointFeedback();
		constraint->enableFeedback(true);
		constraint->setJointFeedback(fb);
    
		//Joint limits damping - avoid explosion
		if(constraint->getConstraintType() != FIXED_CONSTRAINT_TYPE
				&& constraint->getConstraintType() != GEAR_CONSTRAINT_TYPE)
		{
			constraint->setParam(BT_CONSTRAINT_CFM, 0.0);
			constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.0);
			constraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.2);
		}
    
		//Add joint to dynamics world
		sm->getDynamicsWorld()->addConstraint(constraint, !collisionEnabled);
	}
	else if(mbConstraint != NULL)
	{
		sm->getDynamicsWorld()->addMultiBodyConstraint(mbConstraint);
	}
}

}
