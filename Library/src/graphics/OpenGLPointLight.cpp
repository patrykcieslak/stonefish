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
//  OpenGLPointLight.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLPointLight.h"

#include "core/SimulationManager.h"
#include "core/GraphicalSimulationApp.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLCamera.h"

namespace sf
{

OpenGLPointLight::OpenGLPointLight(glm::vec3 position, GLfloat radius, glm::vec3 color, GLfloat illuminance) : OpenGLLight(position, radius, color, illuminance)
{
	lightMesh = OpenGLContent::BuildSphere(getSourceRadius());
	lightObject = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(lightMesh);
}

LightType OpenGLPointLight::getType()
{
    return POINT_LIGHT;
}

void OpenGLPointLight::SetupShader(LightUBO* ubo)
{
    PointLightUBO* pointUbo = (PointLightUBO*)ubo;
    pointUbo->position = getPosition();
    pointUbo->color = getColor();
}
 
void OpenGLPointLight::DrawLight()
{
	OpenGLLight::DrawLight();
	glm::mat4 model = glm::translate(getPosition());
	glm::mat4 view = activeView->GetViewMatrix();
	glm::mat4 proj = activeView->GetProjectionMatrix();
	glm::mat4 MVP = proj * view * model;
	lightSourceShader->SetUniform("MVP", MVP);
	((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawObject(lightObject, 0, glm::mat4());
}
   
}
