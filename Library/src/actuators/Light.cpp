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
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLPointLight.h"
#include "graphics/OpenGLSpotLight.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Light::Light(std::string uniqueName, const Transform& origin, Color color, float illuminance) : LinkActuator(uniqueName)
{
    if(!SimulationApp::getApp()->hasGraphics())
        cCritical("Not possible to use lights in console simulation! Use graphical simulation if possible.");
    
    Vector3 pos = origin.getOrigin();
    glm::vec3 initPos((GLfloat)pos.x(), (GLfloat)pos.y(), (GLfloat)pos.z());
    glLight = new OpenGLPointLight(initPos, color.rgb, illuminance);
    UpdateTransform();
    glLight->UpdateTransform();
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddLight(glLight);
}

Light::Light(std::string uniqueName, const Transform& origin, Scalar coneAngleDeg, Color color, float illuminance) : LinkActuator(uniqueName)
{
    if(!SimulationApp::getApp()->hasGraphics())
        cCritical("Not possible to use lights in console simulation! Use graphical simulation if possible.");
    
    Vector3 pos = origin.getOrigin();
    glm::vec3 initPos((GLfloat)pos.x(), (GLfloat)pos.y(), (GLfloat)pos.z());
    glm::vec3 initDir(0.f, 0.f, 1.f);
    glLight = new OpenGLSpotLight(initPos, initDir, (GLfloat)coneAngleDeg, color.rgb, illuminance);
    UpdateTransform();
    glLight->UpdateTransform();
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddLight(glLight);
}
    
ActuatorType Light::getType()
{
    return ActuatorType::ACTUATOR_LIGHT;
}

void Light::Update(Scalar dt)
{
}

void Light::UpdateTransform()
{
    Transform lightTransform = getActuatorFrame();
    Vector3 pos = lightTransform.getOrigin();
    Quaternion ori = lightTransform.getRotation();
    glm::vec3 glPos((GLfloat)pos.x(), (GLfloat)pos.y(), (GLfloat)pos.z());
    glm::quat glQuat((GLfloat)ori.x(), (GLfloat)ori.y(), (GLfloat)ori.z(), (GLfloat)ori.w());
    glLight->Update(glPos, glQuat);
}

}
