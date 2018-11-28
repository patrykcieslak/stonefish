//
//  Stream.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/07/18.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Stream.h"

using namespace sf;

Stream::Stream(const std::vector<Vector3>& streamline, const std::vector<Scalar>& radius, Scalar inputVelocity, Scalar exponent)
{
    c = streamline;
    r = radius;
    vin = inputVelocity;
    gamma = exponent;
}
    
Vector3 Stream::GetVelocityAtPoint(const Vector3& p)
{
    return Vector3(0,0,0);
}

std::vector<Renderable> Stream::Render()
{
    return std::vector<Renderable>(0);
}
