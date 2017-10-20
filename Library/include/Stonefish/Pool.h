//
//  Pool.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Pool__
#define __Stonefish_Pool__

#include "Liquid.h"
#include "OpenGLPool.h"

class Pool : public Liquid
{
public:
    Pool(std::string uniqueName, Fluid* f);
    
    ForcefieldType getForcefieldType();
    OpenGLPool& getOpenGLPool();
	
private:	
    OpenGLPool glPool;
};
#endif
