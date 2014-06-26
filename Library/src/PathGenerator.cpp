//
//  PathGenerator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 22/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "PathGenerator.h"

#pragma mark Constructors
PathGenerator::PathGenerator()
{
    time = btScalar(0.);
    length = btScalar(0.);
    renderable = false;
}

#pragma mark - Destructor
PathGenerator::~PathGenerator()
{
}

#pragma mark - Accessors
btScalar PathGenerator::getTime()
{
    return time;
}

btScalar PathGenerator::getLength()
{
    return length;
}

#pragma mark - Methods
void PathGenerator::MoveOnPath(btScalar distance, btVector3& point, btVector3& tangent)
{
    if(length > btScalar(0.))
    {
        btScalar dt = distance/length;
        time += dt;
        time = time > btScalar(1.) ? btScalar(1.) : time;
        PointAtTime(time, point, tangent);
    }
}

bool PathGenerator::isRenderable()
{
    return renderable;
}

void PathGenerator::setRenderable(bool render)
{
    renderable = render;
}