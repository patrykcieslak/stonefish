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

OpenGLPointLight::OpenGLPointLight(glm::vec3 position, GLfloat radius, glm::vec3 color, GLfloat lum) : OpenGLLight(position, radius, color, lum)
{
    colorLi = glm::vec4(color, (GLfloat)(lum/(4.f*M_PI)));
	Mesh* m = OpenGLContent::BuildSphere(getSourceRadius());
	sourceObject = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(m);
    delete m;
}

LightType OpenGLPointLight::getType() const
{
    return LightType::POINT;
}

void OpenGLPointLight::SetupShader(LightUBO* ubo)
{
    PointLightUBO* pointUbo = (PointLightUBO*)ubo;
    pointUbo->position = getPosition();
    pointUbo->radius = getSourceRadius();
    pointUbo->color = glm::vec3(colorLi) * colorLi.a; 
}
   
}
