/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
    collisionEnabled = collideLinkedEntities;
	mbConstraint = NULL;
	constraint = NULL;
}

Joint::~Joint(void)
{
    if(SimulationApp::getApp() != NULL)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
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
    
        constraint->setParam(BT_CONSTRAINT_ERP, 0.75);
        constraint->setParam(BT_CONSTRAINT_STOP_ERP, 1.0);
        constraint->setParam(BT_CONSTRAINT_CFM, 0.0);
        constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.0);
        
		//Avoid explosion by softening the joint limits
		if(constraint->getConstraintType() != FIXED_CONSTRAINT_TYPE)
			constraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.2);
    
		//Add joint to dynamics world
		sm->getDynamicsWorld()->addConstraint(constraint, !collisionEnabled);
	}
	else if(mbConstraint != NULL)
	{
		sm->getDynamicsWorld()->addMultiBodyConstraint(mbConstraint);
	}
}
    
void Joint::ApplyDamping()
{
    //Not applicable.
}
    
bool Joint::SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance)
{
    return true; //Nothing to solve
}

std::vector<Renderable> Joint::Render()
{
    std::vector<Renderable> items(0);
    return items;
}
    
}
