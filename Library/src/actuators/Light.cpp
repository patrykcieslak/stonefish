//
//  Light.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "actuators/Light.h"

#include "core/GraphicalSimulationApp.h"
#include "core/Console.h"
#include "graphics/OpenGLPointLight.h"
#include "graphics/OpenGLSpotLight.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Light::Light(std::string uniqueName, const Vector3& initialPos, Color color, float intensity) : LinkActuator(uniqueName)
{
    if(!SimulationApp::getApp()->hasGraphics())
        cCritical("Not possible to use lights in console simulation! Use graphical simulation if possible.");
    
    glLight = new OpenGLPointLight(initialPos, glm::vec4(color.rgb, intensity));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddLight(glLight);
}

Light::Light(std::string uniqueName, const Vector3& initialPos, const Vector3& initialDir, Scalar coneAngle, Color color, float intensity) : LinkActuator(uniqueName)
{
    if(!SimulationApp::getApp()->hasGraphics())
        cCritical("Not possible to use lights in console simulation! Use graphical simulation if possible.");
    
    glLight = new OpenGLSpotLight(initialPos, initialPos+initialDir, (GLfloat)coneAngle, glm::vec4(color.rgb, intensity));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddLight(glLight);
}

void Light::Update(Scalar dt)
{
    //TODO: Update light position when solid moves!!!!
}

}
