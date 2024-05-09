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
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLTrackball.h"

#include "core/GraphicalSimulationApp.h"
#include "entities/SolidEntity.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLState.h"

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
    continuous = true;
    holdingEntity = nullptr;

    outlineShader[0] = new GLSLShader("outline.frag");
    outlineShader[0]->AddUniform("color", ParameterType::VEC4);
    outlineShader[0]->AddUniform("texStencil", ParameterType::INT);
    outlineShader[0]->AddUniform("uvOffset", ParameterType::VEC2);

    outlineShader[1] = new GLSLShader("smallBlur.frag");
    outlineShader[1]->AddUniform("tex", ParameterType::INT);
    outlineShader[1]->AddUniform("invTexSize", ParameterType::VEC2);

    UpdateTransform();
}

OpenGLTrackball::~OpenGLTrackball()
{
    if(outlineShader[0] != nullptr)
        delete outlineShader[0];
    if(outlineShader[1] != nullptr)
        delete outlineShader[1];
}

ViewType OpenGLTrackball::getType()
{
    return ViewType::TRACKBALL;
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

void OpenGLTrackball::UpdateCenterPos()
{
    if(holdingEntity != nullptr)
    {
        Vector3 org = holdingEntity->getOTransform().getOrigin();
        tempCenter = glm::vec3((GLfloat)org.x(), (GLfloat)org.y(), (GLfloat)org.z());
    }
}

void OpenGLTrackball::UpdateTransform()
{
    if(holdingEntity != nullptr) center = tempCenter;
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
            center = translation_start + (GetUpDirection() * (y-y_start) * -0.5f + right * (x-x_start) * -0.5f) * radius;
        }
        else //rotate
        {
            GLfloat z = calculateZ(x, y);
            glm::quat rotation_new = glm::rotation(glm::normalize(glm::vec3(-x_start, z_start, y_start)), glm::normalize(glm::vec3(-x, z, y)));
            rotation = rotation_new * rotation_start;
        }
    }
}

void OpenGLTrackball::MouseScroll(GLfloat s)
{
    radius += s * radius/15.f;
    if(radius < 0.05f) radius = 0.05f;
}

glm::mat4 OpenGLTrackball::GetViewMatrix() const
{
    return trackballTransform;
}

void OpenGLTrackball::Rotate(glm::quat rot)
{
    rotation = rotation * rot;
    UpdateTransform();
}

void OpenGLTrackball::MoveCenter(glm::vec3 step)
{
    center += step;
}

void OpenGLTrackball::GlueToMoving(MovingEntity* ent)
{
    holdingEntity = ent;
}

void OpenGLTrackball::DrawSelection(const std::vector<Renderable>& r, GLuint destinationFBO)
{
    if(r.size() == 0) //No selection
        return;

    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    
    //1. Draw flat shape to color and stencil buffer
    OpenGLState::BindFramebuffer(getRenderFBO());
    SetRenderBuffers(0, false, false);
    OpenGLState::EnableStencilTest();
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    content->SetDrawingMode(DrawingMode::FLAT);
    for(size_t i=0; i<r.size(); ++i)
        if(r[i].type == RenderableType::SOLID)
            content->DrawObject(r[i].objectId, -1, r[i].model);
    OpenGLState::DisableStencilTest();
    
    //2. Grow
    outlineShader[0]->Use();
    outlineShader[0]->SetUniform("color", glm::vec4(1.f,0.55f,0.1f,1.f)); //glm::vec4(0.4f,0.9f,1.f,1.f));
    outlineShader[0]->SetUniform("texStencil", TEX_POSTPROCESS1);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, getColorTexture(0));
    SetRenderBuffers(1, false, false);
    outlineShader[0]->SetUniform("uvOffset", glm::vec2(1.f/(GLfloat)viewportWidth, 1.f/(GLfloat)viewportHeight)*1.5f);
    content->DrawSAQ();

    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, getColorTexture(1));
    SetRenderBuffers(0, false, false);
    content->DrawSAQ();

    //3. Cut outline
    SetRenderBuffers(1, false, true);
    OpenGLState::EnableStencilTest();
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    content->DrawTexturedSAQ(getColorTexture(0));
    OpenGLState::DisableStencilTest();

    //4. Gaussian blur 3x3
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, getColorTexture(1));
    SetRenderBuffers(0, false, false);
    outlineShader[1]->Use();
    outlineShader[1]->SetUniform("tex", TEX_POSTPROCESS1);
    outlineShader[1]->SetUniform("invTexSize", glm::vec2(1.f/(GLfloat)viewportWidth, 1.f/(GLfloat)viewportHeight));
    content->DrawSAQ();

    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, getColorTexture(0));
    SetRenderBuffers(1, false, false);
    content->DrawSAQ();
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::UseProgram(0);

    //5. Overlay
    OpenGLState::BindFramebuffer(destinationFBO); //No depth buffer, just one color buffer
    OpenGLState::EnableBlend();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    content->DrawTexturedSAQ(getColorTexture(1));
    OpenGLState::DisableBlend();
}

}
