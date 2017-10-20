//
//  Pool.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Pool.h"

Pool::Pool(std::string uniqueName, Fluid* f) : Liquid(uniqueName, f)
{
}

ForcefieldType Pool::getForcefieldType()
{
    return FORCEFIELD_POOL;
}

OpenGLPool& Pool::getOpenGLPool()
{
	return glPool;
}