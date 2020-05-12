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
//  OpenGLTrackball.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/29/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLTrackball.h"

#include "core/SimulationApp.h"
#include "entities/SolidEntity.h"

namespace sf
{

OpenGLTrackball::OpenGLTrackball(glm::vec3 centerPosition, GLfloat orbitRadius, glm::vec3 up,
                                 GLint x, GLint y, GLint width, GLint height, GLfloat horizontalFovDeg,
                                 glm::vec2 range) : OpenGLCamera(x, y, width, height, range)
{
    this->up = glm::normalize(up);
    rotation = glm::rotation(this->up, glm::vec3(0,0,1.f));
    center = centerPosition;
    radius = orbitRadius;
    fovx = horizontalFovDeg/180.f*M_PI;
    GLfloat fovy = 2.f * atanf( (GLfloat)viewportHeight/(GLfloat)viewportWidth * tanf(fovx/2.f) );
    projection = glm::perspectiveFov(fovy, (GLfloat)viewportWidth, (GLfloat)viewportHeight, near, far);
    dragging = false;
    transMode = false;
    holdingEntity = NULL;

    UpdateTransform();
}

ViewType OpenGLTrackball::getType()
{
    return TRACKBALL;
}

bool OpenGLTrackball::needsUpdate()
{
    return enabled;
}

glm::vec3 OpenGLTrackball::GetEyePosition() const
{
    return center - radius * GetLookingDirection();
}

glm::vec3 OpenGLTrackball::GetLookingDirection() const
{
    return glm::normalize(glm::vec3(glm::rotate(glm::inverse(rotation), glm::vec4(0,-1.f,0,1.f))));
}

glm::vec3 OpenGLTrackball::GetUpDirection() const
{
    glm::vec3 localUp = glm::vec3(0,0,1.f);	
    return glm::normalize(glm::vec3(glm::rotate(glm::inverse(rotation), glm::vec4(localUp, 1.f))));
}

void OpenGLTrackball::UpdateTransform()
{
    if(holdingEntity != NULL)
    {
        glm::mat4 solidTrans = glMatrixFromTransform(holdingEntity->getCGTransform());
        center = glm::vec3(solidTrans[3]);
    }
    trackballTransform = glm::lookAt(GetEyePosition(), center, GetUpDirection());
    
    viewUBOData.VP = GetProjectionMatrix() * GetViewMatrix();
    viewUBOData.eye = GetEyePosition();
    ExtractFrustumFromVP(viewUBOData.frustum, viewUBOData.VP);
}

GLfloat OpenGLTrackball::calculateZ(GLfloat x, GLfloat y)
{
    if(x*x+y*y <= 0.5f)
        return sqrtf(1.f-(x*x+y*y));
    else
        return 0.5f/sqrtf(x*x+y*y);
}

void OpenGLTrackball::MouseDown(GLfloat x, GLfloat y, bool translate)
{
    x_start = x;
    y_start = y;
    
    if(translate)
    {
        transMode = true;
        translation_start = center;
    }
    else
    {
        transMode = false;
        z_start = calculateZ(x_start, y_start);
        rotation_start = rotation;
    }
    
    dragging = true;
}

void OpenGLTrackball::MouseUp()
{
    dragging = false;
}

void OpenGLTrackball::MouseMove(GLfloat x, GLfloat y)
{
    if(dragging)
    {
        if(transMode)
        {
            glm::vec3 right = glm::normalize(glm::cross(GetLookingDirection(), GetUpDirection()));
            center = translation_start + GetUpDirection() * (y-y_start) * -0.5f + right * (x-x_start) * -0.5f; 
        }
        else //rotate
        {
            GLfloat z = calculateZ(x, y);
            glm::quat rotation_new = glm::rotation(glm::normalize(glm::vec3(-x_start, z_start, y_start)), glm::normalize(glm::vec3(-x, z, y)));
            rotation = rotation_new * rotation_start;
        }
        
        UpdateTransform();
    }
}

void OpenGLTrackball::MouseScroll(GLfloat s)
{
    Scalar factor = pow(radius/5.0, 2.0);
    factor = factor > 1.0 ? 1.0 : factor;
    
    radius += s * factor;
    if(radius < 0.1) radius = 0.1;
    UpdateTransform();
}

glm::mat4 OpenGLTrackball::GetViewMatrix() const
{
    return trackballTransform;
}

void OpenGLTrackball::Rotate(glm::quat rot)
{
    rotation = glm::rotation(up,  glm::vec3(0,0,1.f)) * rot;
    UpdateTransform();
}

void OpenGLTrackball::MoveCenter(glm::vec3 step)
{
    center += step;
    UpdateTransform();
}

void OpenGLTrackball::GlueToEntity(SolidEntity* solid)
{
    holdingEntity = solid;
}

}
