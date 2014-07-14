//
//  Joint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Joint.h"

NameManager Joint::nameManager;

#pragma mark Constructors
Joint::Joint(std::string uniqueName, bool collideLinkedEntities)
{
    name = nameManager.AddName(uniqueName);
    renderable = false;
    collisionEnabled = collideLinkedEntities;
}

#pragma mark - Destructor
Joint::~Joint(void)
{
    nameManager.RemoveName(name);
}

#pragma mark - Accessors
void Joint::setRenderable(bool render)
{
    renderable = render;
}

bool Joint::isRenderable()
{
    return renderable;
}

btTypedConstraint* Joint::getConstraint()
{
    return constraint;
}

std::string Joint::getName()
{
    return name;
}

void Joint::setConstraint(btTypedConstraint *constr)
{
    constraint = constr;
}

#pragma mark - Methods
void Joint::AddToDynamicsWorld(btDynamicsWorld *world)
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
    world->addConstraint(constraint, !collisionEnabled);
}