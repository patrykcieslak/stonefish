//
//  Joint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Joint.h"

Joint::Joint(std::string uniqueName, bool collideLinkedEntities)
{
    name = uniqueName;
    renderable = false;
    collisionEnabled = collideLinkedEntities;
}

Joint::~Joint(void)
{
}

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

void Joint::AddToDynamicsWorld(btDynamicsWorld *world)
{
    btJointFeedback* fb = new btJointFeedback();
    constraint->enableFeedback(true);
    constraint->setJointFeedback(fb);
    
    world->addConstraint(constraint, !collisionEnabled);
}