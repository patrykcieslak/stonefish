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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLTrackball.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLState.h"
#include "graphics/OpenGLRealOcean.h"

namespace sf
{

OpenGLTrackball::OpenGLTrackball(glm::vec3 centerPosition, GLfloat orbitRadius, glm::vec3 up,
                                 GLint x, GLint y, GLint width, GLint height, GLfloat horizontalFovDeg,
                                 glm::vec2 range) : OpenGLCamera(x, y, width, height, range)
{
    this->up_ = glm::normalize(up);
    rotation_ = glm::rotation(this->up_, glm::vec3(0,0,1.f));
    center_ = centerPosition;
    radius_ = orbitRadius;
    fovx_ = horizontalFovDeg/180.f*M_PI;
    GLfloat fovy = 2.f * atanf( (GLfloat)viewportHeight_/(GLfloat)viewportWidth_ * tanf(fovx_/2.f) );
    projection_ = glm::perspectiveFov(fovy, (GLfloat)viewportWidth_, (GLfloat)viewportHeight_, near_, far_);
    dragging_ = false;
    transMode_ = false;
    continuous_ = true;
    holdingEntity_ = nullptr;

    outlineShader_[0] = std::make_unique<GLSLShader>("outline.frag");
    outlineShader_[0]->AddUniform("color", ParameterType::VEC4);
    outlineShader_[0]->AddUniform("texStencil", ParameterType::INT);
    outlineShader_[0]->AddUniform("uvOffset", ParameterType::VEC2);

    outlineShader_[1] = std::make_unique<GLSLShader>("smallBlur.frag");
    outlineShader_[1]->AddUniform("tex", ParameterType::INT);
    outlineShader_[1]->AddUniform("invTexSize", ParameterType::VEC2);

    UpdateTransform();
}

ViewType OpenGLTrackball::getType() const
{
    return ViewType::TRACKBALL;
}

bool OpenGLTrackball::needsUpdate()
{
    return enabled_;
}

glm::vec3 OpenGLTrackball::GetEyePosition() const
{
    return center_ - radius_ * GetLookingDirection();
}

glm::vec3 OpenGLTrackball::GetLookingDirection() const
{
    return glm::normalize(glm::vec3(glm::rotate(glm::inverse(rotation_), glm::vec4(0,-1.f,0,1.f))));
}

glm::vec3 OpenGLTrackball::GetUpDirection() const
{
    glm::vec3 localUp = glm::vec3(0,0,1.f);	
    return glm::normalize(glm::vec3(glm::rotate(glm::inverse(rotation_), glm::vec4(localUp, 1.f))));
}

void OpenGLTrackball::UpdateCenterPos()
{
    if(holdingEntity_ != nullptr)
    {
        Vector3 org = holdingEntity_->getOTransform().getOrigin();
        tempCenter_ = glm::vec3((GLfloat)org.x(), (GLfloat)org.y(), (GLfloat)org.z());
    }
}

void OpenGLTrackball::UpdateTransform()
{
    if(holdingEntity_ != nullptr) center_ = tempCenter_;
    trackballTransform_ = glm::lookAt(GetEyePosition(), center_, GetUpDirection());
    
    viewUBOData_.VP = GetProjectionMatrix() * GetViewMatrix();
    viewUBOData_.eye = GetEyePosition();
    ExtractFrustumFromVP(viewUBOData_.frustum, viewUBOData_.VP);
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
    xStart_ = x;
    yStart_ = y;
    
    if(translate)
    {
        transMode_ = true;
        translationStart_ = center_;
    }
    else
    {
        transMode_ = false;
        zStart_ = calculateZ(xStart_, yStart_);
        rotationStart_ = rotation_;
    }
    
    dragging_ = true;
}

void OpenGLTrackball::MouseUp()
{
    dragging_ = false;
}

void OpenGLTrackball::MouseMove(GLfloat x, GLfloat y)
{
    if(dragging_)
    {
        if(transMode_)
        {
            glm::vec3 right = glm::normalize(glm::cross(GetLookingDirection(), GetUpDirection()));
            center_ = translationStart_ + (GetUpDirection() * (y-yStart_) * -0.5f + right * (x-xStart_) * -0.5f) * radius_;
        }
        else //rotate
        {
            GLfloat z = calculateZ(x, y);
            glm::quat rotation_new = glm::rotation(glm::normalize(glm::vec3(-xStart_, zStart_, yStart_)), glm::normalize(glm::vec3(-x, z, y)));
            rotation_ = rotation_new * rotationStart_;
        }
    }
}

void OpenGLTrackball::MouseScroll(GLfloat s)
{
    radius_ += s * radius_/15.f;
    if(radius_ < 0.05f) radius_ = 0.05f;
}

glm::mat4 OpenGLTrackball::GetViewMatrix() const
{
    return trackballTransform_;
}

void OpenGLTrackball::Rotate(glm::quat rot)
{
    rotation_ = glm::rotation(up_,  glm::vec3(0,0,1.f)) * rot;
}

void OpenGLTrackball::MoveCenter(glm::vec3 step)
{
    center_ += step;
}

void OpenGLTrackball::GlueToMoving(MovingEntity* ent)
{
    holdingEntity_ = ent;

    //Clear ocean quadtree to avoid holes in the ocean rendering because of the sudden jump of camera origin
    Ocean* ocean = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getSimulationManager()->getOcean();
    if(ocean != nullptr && ocean->hasWaves())
        ((OpenGLRealOcean*)ocean->getOpenGLOcean())->ResetSurface(this);
}

void OpenGLTrackball::DrawSelection(const std::vector<Renderable>& r, GLuint destinationFBO)
{
    if(r.size() == 0) //No selection
        return;

    OpenGLContent* content = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent();
    
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
    outlineShader_[0]->Use();
    outlineShader_[0]->SetUniform("color", glm::vec4(1.f,0.55f,0.1f,1.f)); //glm::vec4(0.4f,0.9f,1.f,1.f));
    outlineShader_[0]->SetUniform("texStencil", TEX_POSTPROCESS1);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, getColorTexture(0));
    SetRenderBuffers(1, false, false);
    outlineShader_[0]->SetUniform("uvOffset", glm::vec2(1.f/(GLfloat)viewportWidth_, 1.f/(GLfloat)viewportHeight_)*1.5f);
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
    outlineShader_[1]->Use();
    outlineShader_[1]->SetUniform("tex", TEX_POSTPROCESS1);
    outlineShader_[1]->SetUniform("invTexSize", glm::vec2(1.f/(GLfloat)viewportWidth_, 1.f/(GLfloat)viewportHeight_));
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
