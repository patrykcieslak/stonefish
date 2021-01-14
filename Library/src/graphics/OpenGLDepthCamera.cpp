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
//  OpenGLDepthCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 04/05/18.
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLDepthCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

GLSLShader** OpenGLDepthCamera::depthCameraOutputShader = NULL;
GLSLShader* OpenGLDepthCamera::depthVisualizeShader = NULL;

OpenGLDepthCamera::OpenGLDepthCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                                     GLint originX, GLint originY, GLint width, GLint height,
                                     GLfloat horizontalFOVDeg, GLfloat minDepth, GLfloat maxDepth,
                                     bool continuousUpdate, bool useRanges, GLfloat verticalFOVDeg)
 : OpenGLView(originX, originY, width, height), randDist(0.f, 1.f)
{
    _needsUpdate = false;
    continuous = continuousUpdate;
    newData = false;
    camera = NULL;
    noiseDepth = 0.f;
    idx = 0;
    range.x = minDepth;
    range.y = maxDepth;
    usesRanges = useRanges;
    linearDepthPBO = 0;
    
    SetupCamera(eyePosition, direction, cameraUp);
    UpdateTransform();
    
    GLfloat fovx = horizontalFOVDeg/180.f*M_PI;
    
    if(verticalFOVDeg > 0.f)
    {
        GLfloat fovy = verticalFOVDeg/180.f*M_PI;
        projection[0] = glm::vec4(range.x/(range.x*tanf(fovx/2.f)), 0.f, 0.f, 0.f);
        projection[1] = glm::vec4(0.f, range.x/(range.x*tanf(fovy/2.f)), 0.f, 0.f);
        projection[2] = glm::vec4(0.f, 0.f, -(range.y + range.x)/(range.y-range.x), -1.f);
        projection[3] = glm::vec4(0.f, 0.f, -2.f*range.y*range.x/(range.y-range.x), 0.f);
    }
    else
    {
        GLfloat fovy = 2.f * atanf( (GLfloat)viewportHeight/(GLfloat)viewportWidth * tanf(fovx/2.f) );
        projection = glm::perspectiveFov(fovy, (GLfloat)viewportWidth, (GLfloat)viewportHeight, range.x, range.y);
    }
    
    //Render depth
    glGenTextures(1, &renderDepthTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, renderDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, viewportWidth, viewportHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    OpenGLState::UnbindTexture(TEX_BASE);

    glGenFramebuffers(1, &renderFBO);
    OpenGLState::BindFramebuffer(renderFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderDepthTex, 0);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Depth Render FBO initialization failed!");
    
    //Linear depth
    glGenTextures(1, &linearDepthTex);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, linearDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, viewportWidth, viewportHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    OpenGLState::UnbindTexture(TEX_BASE);

    glGenFramebuffers(1, &linearDepthFBO);
    OpenGLState::BindFramebuffer(linearDepthFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, linearDepthTex, 0);
        
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Linear Depth FBO initialization failed!");
    
    OpenGLState::BindFramebuffer(0);
}

OpenGLDepthCamera::~OpenGLDepthCamera()
{
    glDeleteTextures(1, &renderDepthTex);
    glDeleteFramebuffers(1, &renderFBO);
    glDeleteTextures(1, &linearDepthTex);
    glDeleteFramebuffers(1, &linearDepthFBO);

    if(camera != NULL)
    {
        glDeleteBuffers(1, &linearDepthPBO);
    }
}

void OpenGLDepthCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir = _dir;
    tempEye = _eye;
    tempUp = _up;
}

void OpenGLDepthCamera::UpdateTransform()
{
    eye = tempEye;
    dir = tempDir;
    up = tempUp;
    SetupCamera();

    //Inform camera to run callback
    if(newData)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, linearDepthPBO);
        GLfloat* src = (GLfloat*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            camera->NewDataReady(src, idx);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData = false;
    }
}

void OpenGLDepthCamera::SetupCamera()
{
    cameraTransform = glm::lookAt(eye, eye+dir, up);
}

glm::vec3 OpenGLDepthCamera::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLDepthCamera::GetLookingDirection() const
{
    return dir;
}

glm::vec3 OpenGLDepthCamera::GetUpDirection() const
{
    return up;
}

glm::mat4 OpenGLDepthCamera::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLDepthCamera::GetViewMatrix() const
{
    return cameraTransform;
}

GLfloat OpenGLDepthCamera::GetFarClip() const
{
    return range.y;
}

void OpenGLDepthCamera::Update()
{
    _needsUpdate = true;
}

bool OpenGLDepthCamera::needsUpdate()
{
    if(_needsUpdate)
    {
        _needsUpdate = false;
        return enabled;
    }
    else
        return false;
}

void OpenGLDepthCamera::setCamera(Camera* cam, unsigned int index)
{
    camera = cam;
    idx = index;

    glGenBuffers(1, &linearDepthPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, linearDepthPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * sizeof(GLfloat), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void OpenGLDepthCamera::setNoise(GLfloat depthStdDev)
{
    noiseDepth = depthStdDev;
}

ViewType OpenGLDepthCamera::getType()
{
    return ViewType::DEPTH_CAMERA;
}

void OpenGLDepthCamera::ComputeOutput(std::vector<Renderable>& objects)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetCurrentView(this);
    content->SetDrawingMode(DrawingMode::SHADOW);
    OpenGLState::BindFramebuffer(renderFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_CLAMP);
    for(size_t h=0; h<objects.size(); ++h)
    {
        if(objects[h].type != RenderableType::SOLID)
            continue;
        content->DrawObject(objects[h].objectId, -1, objects[h].model);
    }
    glEnable(GL_DEPTH_CLAMP);
    OpenGLState::BindFramebuffer(0);
}

void OpenGLDepthCamera::LinearizeDepth()
{
    OpenGLState::BindFramebuffer(linearDepthFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderDepthTex);
    depthCameraOutputShader[0]->Use();
    depthCameraOutputShader[0]->SetUniform("texDepth", TEX_POSTPROCESS1);
    depthCameraOutputShader[0]->SetUniform("rangeInfo", glm::vec4(range.x, range.y, range.x*range.y, range.x-range.y));
    depthCameraOutputShader[0]->SetUniform("noiseSeed", glm::vec3(randDist(randGen), randDist(randGen), randDist(randGen)));
    depthCameraOutputShader[0]->SetUniform("noiseStddev", noiseDepth);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::BindFramebuffer(0);
}
    
void OpenGLDepthCamera::Depth2LinearRanges()
{
    glm::mat4 proj = GetProjectionMatrix();
    glm::vec4 projInfo(
                       2.0f/proj[0].x,
                       2.0f/proj[1].y,
                       -(1.f-proj[0].z)/proj[0].x,
                       -(1.f+proj[1].z)/proj[1].y
                       );
    
    OpenGLState::BindFramebuffer(linearDepthFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderDepthTex);
    depthCameraOutputShader[1]->Use();
    depthCameraOutputShader[1]->SetUniform("projInfo", projInfo);
    depthCameraOutputShader[1]->SetUniform("rangeInfo", glm::vec4(range.x, range.y, range.x*range.y, range.x-range.y));
    depthCameraOutputShader[1]->SetUniform("texDepth", TEX_POSTPROCESS1);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    OpenGLState::BindFramebuffer(0);
}

void OpenGLDepthCamera::DrawLDR(GLuint destinationFBO, bool updated)
{
    //Check if there is a need to display image on screen
    bool display = true;
    unsigned int dispX, dispY;
    GLfloat dispScale;
    if(camera != NULL)
        display = camera->getDisplayOnScreen(dispX, dispY, dispScale);
    
    //Draw on screen
    if(display)
    {
        if(usesRanges) Depth2LinearRanges();
        else LinearizeDepth();
        
        //Bind depth texture
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, linearDepthTex);
        //LDR drawing
        OpenGLState::BindFramebuffer(destinationFBO);
        OpenGLState::Viewport(originX, originY, viewportWidth, viewportHeight);
        depthVisualizeShader->Use();
        depthVisualizeShader->SetUniform("texLinearDepth", TEX_POSTPROCESS1);
        depthVisualizeShader->SetUniform("range", range);
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
        OpenGLState::BindFramebuffer(0);
        OpenGLState::UseProgram(0);
        //Unbind textures
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
    }
    
    //Copy texture to camera buffer
    if(camera != NULL && updated)
    {
        if(!display)
        {
            if(usesRanges) Depth2LinearRanges();
            else LinearizeDepth();
        }
                
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, linearDepthTex);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, linearDepthPBO);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        newData = true;
    }
}

///////////////////////// Static /////////////////////////////
void OpenGLDepthCamera::Init()
{
    depthCameraOutputShader = new GLSLShader*[2];
    depthCameraOutputShader[0] = new GLSLShader("depthCameraOutput.frag");
    depthCameraOutputShader[0]->AddUniform("rangeInfo", ParameterType::VEC4);
    depthCameraOutputShader[0]->AddUniform("texDepth", ParameterType::INT);
    depthCameraOutputShader[0]->AddUniform("noiseSeed", ParameterType::VEC3);
    depthCameraOutputShader[0]->AddUniform("noiseStddev", ParameterType::FLOAT);
    
    depthCameraOutputShader[1] = new GLSLShader("depthCameraOutput2.frag");
    depthCameraOutputShader[1]->AddUniform("projInfo", ParameterType::VEC4);
    depthCameraOutputShader[1]->AddUniform("rangeInfo", ParameterType::VEC4);
    depthCameraOutputShader[1]->AddUniform("texDepth", ParameterType::INT);
    
    depthVisualizeShader = new GLSLShader("depthVisualize.frag");
    depthVisualizeShader->AddUniform("range", ParameterType::VEC2);
    depthVisualizeShader->AddUniform("texLinearDepth", ParameterType::INT);
}

void OpenGLDepthCamera::Destroy()
{
    if(depthCameraOutputShader != NULL) 
    {
        if(depthCameraOutputShader[0] != NULL) delete depthCameraOutputShader[0];
        if(depthCameraOutputShader[1] != NULL) delete depthCameraOutputShader[1];
        delete [] depthCameraOutputShader;
    }
    if(depthVisualizeShader != NULL) delete depthVisualizeShader;
}

}
