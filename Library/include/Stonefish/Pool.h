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
    //Pool
    Pool(std::string uniqueName, Fluid* f);
	virtual ~Pool();
	
    void setAlgeaBloomFactor(GLfloat f);
    void setTurbidity(GLfloat ntu);
    GLfloat getAlgeaBloomFactor();
    GLfloat getTurbidity();
    OpenGLPool* getOpenGLPool();
	
    //Force field
    ForcefieldType getForcefieldType();
    
private:	
    glm::vec3 ComputeLightAbsorption();
	
    OpenGLPool* glPool;
    GLfloat algeaBloom;
    GLfloat turbidity;
};
#endif
