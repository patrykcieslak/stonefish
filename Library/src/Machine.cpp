//
//  Machine.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Machine.h"

Machine::Machine(std::string uniqueName)
{
    name = uniqueName;
}

Machine::~Machine(void)
{
    
}

std::string Machine::getName()
{
    return name;
}

