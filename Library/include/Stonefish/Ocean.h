//
//  Ocean.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Ocean__
#define __Stonefish_Ocean__

#include "Liquid.h"
#include "OpenGLOcean.h"

class Ocean : public Liquid
{
public:
    Ocean(std::string uniqueName, Fluid* f);
    virtual ~Ocean();
	
    ForcefieldType getForcefieldType();
    OpenGLOcean* getOpenGLOcean();
	
private:	
    OpenGLOcean* glOcean;
};
#endif
