//
//  Stream.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Stream.h"

using namespace sf;

Stream::Stream(const std::vector<btVector3>& streamline, const std::vector<btScalar>& radius, btScalar inputVelocity, btScalar exponent)
{
    c = streamline;
    r = radius;
    vin = inputVelocity;
    gamma = exponent;
}
    
btVector3 Stream::GetVelocityAtPoint(const btVector3& p)
{
    return btVector3(0,0,0);
}

std::vector<Renderable> Stream::Render()
{
    return std::vector<Renderable>(0);
}
