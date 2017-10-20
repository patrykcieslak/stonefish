//
//  Ocean.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Ocean.h"

Ocean::Ocean(std::string uniqueName, Fluid* f) : Liquid(uniqueName, f)
{
}

ForcefieldType Ocean::getForcefieldType()
{
    return FORCEFIELD_OCEAN;
}

OpenGLOcean& Ocean::getOpenGLOcean()
{
	return glOcean;
}