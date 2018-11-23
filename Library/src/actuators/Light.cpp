//
//  Light.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "actuators/Light.h"

#include "graphics/OpenGLPointLight.h"
#include "graphics/OpenGLSpotLight.h"

using namespace sf;

Light::Light(std::string uniqueName, const btVector3& initialPos, glm::vec4 color) : LinkActuator(uniqueName)
{
    glLight = new OpenGLPointLight(initialPos, color);
    OpenGLContent::getInstance()->AddLight(glLight);
}

Light::Light(std::string uniqueName, const btVector3& initialPos, const btVector3& initialDir, btScalar coneAngle, glm::vec4 color) : LinkActuator(uniqueName)
{
    glLight = new OpenGLSpotLight(initialPos, initialPos+initialDir, (GLfloat)coneAngle, color);
    OpenGLContent::getInstance()->AddLight(glLight);
}

void Light::Update(btScalar dt)
{
    //TODO: Update light position when solid moves!!!!
}
