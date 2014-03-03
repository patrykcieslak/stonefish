//
//  Joint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Joint.h"

Joint::Joint()
{
    renderable = true;
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

void Joint::setConstraint(btTypedConstraint *constr)
{
    constraint = constr;
}

void Joint::AddToDynamicsWorld(btDynamicsWorld *world)
{
    world->addConstraint(constraint);
}