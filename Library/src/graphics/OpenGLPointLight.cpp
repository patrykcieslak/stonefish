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
#include "graphics/GLSLShader.h"

namespace sf
{

OpenGLPointLight::OpenGLPointLight(glm::vec3 position, glm::vec3 color, GLfloat illuminance) : OpenGLLight(position, color, illuminance)
{
}

LightType OpenGLPointLight::getType()
{
    return POINT_LIGHT;
}

void OpenGLPointLight::SetupShader(GLSLShader* shader, unsigned int lightId)
{
    std::string lightUni = "pointLights[" + std::to_string(lightId) + "].";
    shader->SetUniform(lightUni + "position", getPosition());
    shader->SetUniform(lightUni + "color", getColor());
}
    
}
