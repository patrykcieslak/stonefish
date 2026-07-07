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
//  Copyright (c) 2013-2024 Patryk Cieslak. All rights reserved.
//

#include "joints/Joint.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"

namespace sf
{

Joint::Joint(std::string uniqueName, bool collideLinkedEntities)
{
    name_ = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    collisionEnabled_ = collideLinkedEntities;
    mbConstraint_ = nullptr;
    constraint_ = nullptr;
    jSolidA_ = nullptr;
    jSolidB_ = nullptr;
}

Joint::~Joint(void)
{
    if(SimulationApp::getApp() != nullptr)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name_);
}

bool Joint::isMultibodyJoint()
{
    return (constraint_ == nullptr) && (mbConstraint_ != nullptr);
}

btTypedConstraint* Joint::getConstraint()
{
    return constraint_;
}

std::string Joint::getName() const
{
    return name_;
}

void Joint::setConstraint(btTypedConstraint *c)
{
    constraint_ = c;
}

void Joint::setConstraint(btMultiBodyConstraint *c)
{
    mbConstraint_ = c;
}

Scalar Joint::getFeedback(unsigned int dof)
{
    if(dof > 5)
        return Scalar(0);
    
    if(constraint_ != nullptr)
    {
        btJointFeedback* fb = constraint_->getJointFeedback();
        if(dof < 3)
            return fb->m_appliedForceBodyA[dof];
        else
            return fb->m_appliedTorqueBodyA[dof-3];
    }
    else if(mbConstraint_ != nullptr)
    {
        return mbConstraint_->getAppliedImpulse(dof);
    }
    else
        return Scalar(0);
}

SolidEntity* Joint::getSolidA()
{
    return jSolidA_;
}

SolidEntity* Joint::getSolidB()
{
    return jSolidB_;
}

void Joint::AddToSimulation(SimulationManager* sm)
{
    if(constraint_ != nullptr)
    {
        //Force feedback
        btJointFeedback* fb = new btJointFeedback();
        constraint_->enableFeedback(true);
        constraint_->setJointFeedback(fb);

        //Breaking
        constraint_->setBreakingImpulseThreshold(BT_LARGE_FLOAT);

        //Solver setup
        Scalar erp, stopErp;
        sm->getJointErp(erp, stopErp);

        if(constraint_->getConstraintType() == D6_SPRING_2_CONSTRAINT_TYPE)
        {
            for(int i=0; i<6; ++i) // Go through all axes
            {
                constraint_->setParam(BT_CONSTRAINT_ERP, erp, i);
                constraint_->setParam(BT_CONSTRAINT_STOP_ERP, stopErp, i);
                constraint_->setParam(BT_CONSTRAINT_CFM, 0.0, i);
                constraint_->setParam(BT_CONSTRAINT_STOP_CFM, 0.0, i);
            }
        }
        else if (constraint_->getConstraintType() == D6_CONSTRAINT_TYPE) // Generic 6DOF constraint does not support ERP
        {
            for(int i=0; i<6; ++i) // Go through all axes
            {
                constraint_->setParam(BT_CONSTRAINT_STOP_ERP, stopErp, i);
                constraint_->setParam(BT_CONSTRAINT_CFM, 0.0, i);
                constraint_->setParam(BT_CONSTRAINT_STOP_CFM, 0.0, i);
            }
        }
        else // Use default axis
        {
            constraint_->setParam(BT_CONSTRAINT_ERP, erp);
            constraint_->setParam(BT_CONSTRAINT_STOP_ERP, stopErp); //Avoid explosion by softening the joint limits
            constraint_->setParam(BT_CONSTRAINT_CFM, 0.0);
            constraint_->setParam(BT_CONSTRAINT_STOP_CFM, 0.0);
        }
    
        //Add joint to dynamics world
        sm->getDynamicsWorld()->addConstraint(constraint_, !collisionEnabled_);
    }
    else if(mbConstraint_ != nullptr)
    {
        Scalar erp, stopErp;
        sm->getJointErp(erp, stopErp);
        mbConstraint_->setErp(erp);
        sm->getDynamicsWorld()->addMultiBodyConstraint(mbConstraint_);
        if(!collisionEnabled_)
            SimulationApp::getApp()->getSimulationManager()->DisableCollision(jSolidA_, jSolidB_);
    }
}

void Joint::RemoveFromSimulation(SimulationManager* sm)
{
    if(constraint_ != nullptr)
    {
        delete constraint_->getJointFeedback();
        sm->getDynamicsWorld()->removeConstraint(constraint_);
    }
    else if(mbConstraint_ != nullptr)
    {
        sm->getDynamicsWorld()->removeMultiBodyConstraint(mbConstraint_);
        if(!collisionEnabled_)
            SimulationApp::getApp()->getSimulationManager()->EnableCollision(jSolidA_, jSolidB_);
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
