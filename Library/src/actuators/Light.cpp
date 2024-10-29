/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  Light.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2023 Patryk Cieslak. All rights reserved.
//

#include "actuators/Light.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLPointLight.h"
#include "graphics/OpenGLSpotLight.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"
#include "entities/StaticEntity.h"
#include "entities/AnimatedEntity.h"

namespace sf
{

Light::Light(std::string uniqueName, Scalar radius, Color color, Scalar lum) 
	: LinkActuator(uniqueName), attach2(nullptr), attach3(nullptr), c(color), coneAngle(0), glLight(nullptr)
{
    if(!SimulationApp::getApp()->hasGraphics())
        cCritical("Not possible to use lights in console simulation! Use graphical simulation if possible.");
    
	R = radius;
    Fi = lum < Scalar(0) ? Scalar(0) : lum;
}

Light::Light(std::string uniqueName, Scalar radius, Scalar coneAngleDeg, Color color, Scalar lum) 
	: Light(uniqueName, radius, color, lum)
{
    coneAngle = coneAngleDeg > Scalar(0) ? coneAngleDeg : Scalar(45);
}
    
ActuatorType Light::getType() const
{
    return ActuatorType::LIGHT;
}

Transform Light::getActuatorFrame() const
{
	if(attach != nullptr)
        return attach->getOTransform() * o2a; //Solid
    else if(attach2 != nullptr)
		return attach2->getTransform() * o2a; //Static
	else if(attach3 != nullptr)
        return attach3->getOTransform() * o2a; //Animated
    else
        return o2a;
}

void Light::AttachToWorld(const Transform& origin)
{
	o2a = origin;
    attach = nullptr;
    attach2 = nullptr;
    attach3 = nullptr;
    if(glLight == nullptr)
	    InitGraphics();
}
        
void Light::AttachToStatic(StaticEntity* body, const Transform& origin)
{
	if(body != nullptr)
	{
		o2a = origin;
		attach = nullptr;
        attach2 = body;
        attach3 = nullptr;
        if(glLight == nullptr)
		    InitGraphics();
	}
}

void Light::AttachToAnimated(AnimatedEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        o2a = origin;
        attach = nullptr;
        attach2 = nullptr;
        attach3 = body;
        if(glLight == nullptr)
            InitGraphics();
    }
}

void Light::AttachToSolid(SolidEntity* body, const Transform& origin)
{
    LinkActuator::AttachToSolid(body, origin);
    if(body != nullptr)
    {
        attach2 = nullptr;
        attach3 = nullptr;
        if(glLight == nullptr)
            InitGraphics();
    }
}

void Light::InitGraphics()
{
    if(coneAngle > Scalar(0)) //Spot light
        glLight = new OpenGLSpotLight(glm::vec3(0.f), glm::vec3(0.f,0.f,-1.f), (GLfloat)R, (GLfloat)coneAngle, c.rgb, (GLfloat)Fi);
    else //Omnidirectional light
        glLight = new OpenGLPointLight(glm::vec3(0.f), (GLfloat)R, c.rgb, (GLfloat)Fi);
    
    UpdateTransform();
    glLight->UpdateTransform();
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->AddLight(glLight);
}
    
void Light::Update(Scalar dt)
{
}

void Light::UpdateTransform()
{
    Transform lightTransform = getActuatorFrame();
    Vector3 pos = lightTransform.getOrigin();
    glm::vec3 glPos((GLfloat)pos.x(), (GLfloat)pos.y(), (GLfloat)pos.z());
    glLight->UpdatePosition(glPos);
    
    if(coneAngle > Scalar(0))
    {
        Vector3 dir = lightTransform.getBasis().getColumn(2);
        glm::vec3 glDir((GLfloat)dir.x(), (GLfloat)dir.y(), (GLfloat)dir.z());
        ((OpenGLSpotLight*)glLight)->UpdateDirection(glDir);
    }
}
    
std::vector<Renderable> Light::Render()
{
    std::vector<Renderable> items(0);
    //glLight->SwitchOff();
    Renderable item;
    item.model = glMatrixFromTransform(getActuatorFrame());
    item.type = RenderableType::ACTUATOR_LINES;
    
    GLfloat iconSize = 1.f;
    unsigned int div = 24;
    
    if(coneAngle > Scalar(0))
    {
        GLfloat r = iconSize * tanf((GLfloat)coneAngle/360.f*M_PI);
        
        for(unsigned int i=0; i<div; ++i)
        {
            GLfloat angle1 = (GLfloat)i/(GLfloat)div * 2.f * M_PI;
            GLfloat angle2 = (GLfloat)(i+1)/(GLfloat)div * 2.f * M_PI;
            item.points.push_back(glm::vec3(r * cosf(angle1), r * sinf(angle1), iconSize));
            item.points.push_back(glm::vec3(r * cosf(angle2), r * sinf(angle2), iconSize));
        }
        
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(r, 0, iconSize));
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(-r, 0, iconSize));
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(0, r, iconSize));
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(0, -r, iconSize));
    }
    else
    {
        for(unsigned int i=0; i<div; ++i)
        {
            GLfloat angle1 = (GLfloat)i/(GLfloat)div * 2.f * M_PI;
            GLfloat angle2 = (GLfloat)(i+1)/(GLfloat)div * 2.f * M_PI;
            item.points.push_back(glm::vec3(0.5f * iconSize * cosf(angle1), 0.5f * iconSize * sinf(angle1), 0));
            item.points.push_back(glm::vec3(0.5f * iconSize * cosf(angle2), 0.5f * iconSize * sinf(angle2), 0));
            item.points.push_back(glm::vec3(0.5f * iconSize * cosf(angle1), 0, 0.5f * iconSize * sinf(angle1)));
            item.points.push_back(glm::vec3(0.5f * iconSize * cosf(angle2), 0, 0.5f * iconSize * sinf(angle2)));
            item.points.push_back(glm::vec3(0, 0.5f * iconSize * cosf(angle1), 0.5f * iconSize * sinf(angle1)));
            item.points.push_back(glm::vec3(0, 0.5f * iconSize * cosf(angle2), 0.5f * iconSize * sinf(angle2)));
        }
    }
    
    items.push_back(item);
    
    return items;
}

OpenGLLight* Light::getGLLight(){
    return glLight;
}

}
