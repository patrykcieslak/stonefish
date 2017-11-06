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
    algeaBloom = 0.2f;
    turbidity = 100.f;
    glPool.setLightAbsorptionCoeff(ComputeLightAbsorption());
}

ForcefieldType Pool::getForcefieldType()
{
    return FORCEFIELD_POOL;
}

OpenGLPool& Pool::getOpenGLPool()
{
	return glPool;
}

void Pool::setAlgeaBloomFactor(GLfloat f)
{
    algeaBloom = f < 0.f ? 0.f : (f > 1.f ? 1.f : f);
    glPool.setLightAbsorptionCoeff(ComputeLightAbsorption());
}

void Pool::setTurbidity(GLfloat ntu)
{
    turbidity = ntu < 0.f ? 0.f : ntu;
    glPool.setLightAbsorptionCoeff(ComputeLightAbsorption());
}
    
GLfloat Pool::getAlgeaBloomFactor()
{
    return algeaBloom;
}

GLfloat Pool::getTurbidity()
{
    return turbidity;
}

glm::vec3 Pool::ComputeLightAbsorption()
{
    glm::vec3 absorption = glm::vec3(0.2f+0.2f*(algeaBloom), 0.02f, 0.02f+0.02f*(algeaBloom));
    return expf(turbidity/1000.f)*absorption;
}