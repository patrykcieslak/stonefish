//
//  Atmosphere.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 02/12/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Atmosphere.h"

namespace sf
{
    
Atmosphere::Atmosphere(std::string uniqueName, Fluid* f) : ForcefieldEntity(uniqueName)
{
    ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    
    Scalar size(10000);
    Vector3 halfExtents = Vector3(size/Scalar(2), size/Scalar(2), size/Scalar(2));
    ghost->setWorldTransform(Transform(Quaternion::getIdentity(), Vector3(0,0,-size/Scalar(2)))); //Above ocean surface
    ghost->setCollisionShape(new btBoxShape(halfExtents));
    
    gas = f;
    glAtmosphere = NULL;
}
    
Atmosphere::~Atmosphere()
{
    if(glAtmosphere != NULL) delete glAtmosphere;
}
    
OpenGLAtmosphere* Atmosphere::getOpenGLAtmosphere()
{
    return glAtmosphere;
}
    
void Atmosphere::InitGraphics(const RenderSettings& s)
{
    glAtmosphere = new OpenGLAtmosphere(s.atmosphere, s.shadows);
}

ForcefieldType Atmosphere::getForcefieldType()
{
    return ForcefieldType::FORCEFIELD_ATMOSPHERE;
}

}
