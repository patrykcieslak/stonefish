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
//  OpenGLSegmentationCamera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 14/03/2025.
//  Copyright (c) 2025 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLSegmentationCamera.h"

#include "core/GraphicalSimulationApp.h"
#include "entities/SolidEntity.h"
#include "sensors/vision/Camera.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "entities/forcefields/Ocean.h"

namespace sf
{

GLSLShader* OpenGLSegmentationCamera::segmentationCameraOutputShader = nullptr;
GLSLShader* OpenGLSegmentationCamera::segmentationVisualizeShader = nullptr;
GLSLShader* OpenGLSegmentationCamera::flipShader = nullptr;

OpenGLSegmentationCamera::OpenGLSegmentationCamera(glm::vec3 eyePosition, glm::vec3 direction, glm::vec3 cameraUp,
                          GLint originX, GLint originY, GLint width, GLint height,
                          GLfloat horizontalFOVDeg, glm::vec2 range, bool continuousUpdate)
 : OpenGLView(originX, originY, width, height)
{
    _needsUpdate = false;
    continuous = continuousUpdate;
    newData = false;
    camera = nullptr;
    this->range = range;
    
    SetupCamera(eyePosition, direction, cameraUp);
    UpdateTransform();
    
    fov.x = horizontalFOVDeg/180.f*M_PI;
    fov.y = 2.f * atanf( (GLfloat)viewportHeight/(GLfloat)viewportWidth * tanf(fov.x/2.f) );
    projection = glm::perspectiveFov(fov.y, (GLfloat)viewportWidth, (GLfloat)viewportHeight, range.x, range.y);
    focalLength = ((GLfloat)viewportWidth/2.f)/tanf(fov.x/2.f);
    
    //Direct segmentation output
    renderSegTex[0] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                            GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, NULL, FilteringMode::NEAREST, false);
    renderSegTex[1] = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                            GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, NULL, FilteringMode::NEAREST, false);
    renderDepthTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), 
                                                           GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, FilteringMode::NEAREST, false);
    std::vector<FBOTexture> fboTextures;
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderSegTex[0]));
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderSegTex[1]));
    fboTextures.push_back(FBOTexture(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderDepthTex));
    renderFBO = OpenGLContent::GenerateFramebuffer(fboTextures);

    //Segmentation visualization with a color map
    displaySegTex = OpenGLContent::GenerateTexture(GL_TEXTURE_2D, glm::uvec3(viewportWidth, viewportHeight, 0), GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, FilteringMode::BILINEAR, false);

    fboTextures.clear();
    fboTextures.push_back(FBOTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displaySegTex));
    displayFBO = OpenGLContent::GenerateFramebuffer(fboTextures);

    //Display quad
    GLfloat quadData[4][4];
    quadData[0][0] = -1.f;
    quadData[0][1] = 1.f;
    quadData[0][2] = 0.f;
    quadData[0][3] = 1.f;
    quadData[1][0] = -1.f;
    quadData[1][1] = -1.f;
    quadData[1][2] = 0.f;
    quadData[1][3] = 0.f;
    quadData[2][0] = 1.f;
    quadData[2][1] = 1.f;
    quadData[2][2] = 1.f;
    quadData[2][3] = 1.f;
    quadData[3][0] = 1.f;
    quadData[3][1] = -1.f;
    quadData[3][2] = 1.f;
    quadData[3][3] = 0.f;
    
    glGenVertexArrays(1, &displayVAO);
    OpenGLState::BindVertexArray(displayVAO);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &displayVBO);
    glBindBuffer(GL_ARRAY_BUFFER, displayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGLState::BindVertexArray(0);
}

OpenGLSegmentationCamera::~OpenGLSegmentationCamera()
{
    glDeleteTextures(1, &renderDepthTex);
    glDeleteTextures(2, renderSegTex);
    glDeleteTextures(1, &displaySegTex);
    glDeleteFramebuffers(1, &renderFBO);
    glDeleteFramebuffers(1, &displayFBO);
    glDeleteVertexArrays(1, &displayVAO);
    glDeleteBuffers(1, &displayVBO);

    if(camera != nullptr)
    {
        glDeleteBuffers(1, &outputPBO);
        glDeleteBuffers(1, &displayPBO);
    }
}

void OpenGLSegmentationCamera::SetupCamera(glm::vec3 _eye, glm::vec3 _dir, glm::vec3 _up)
{
    tempDir = _dir;
    tempEye = _eye;
    tempUp = _up;
}

void OpenGLSegmentationCamera::UpdateTransform()
{
    eye = tempEye;
    dir = tempDir;
    up = tempUp;
    SetupCamera();

    //Inform camera to run callback
    if(newData)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
        GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            camera->NewDataReady(src, 0);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
        GLushort* src2 = (GLushort*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src2)
        {
            camera->NewDataReady(src2, 1);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //Release pointer to the mapped buffer
        }
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        newData = false;
    }
}

void OpenGLSegmentationCamera::SetupCamera()
{
    cameraTransform = glm::lookAt(eye, eye+dir, up);
}

glm::vec3 OpenGLSegmentationCamera::GetEyePosition() const
{
    return eye;
}

glm::vec3 OpenGLSegmentationCamera::GetLookingDirection() const
{
    return dir;
}

glm::vec3 OpenGLSegmentationCamera::GetUpDirection() const
{
    return up;
}

glm::mat4 OpenGLSegmentationCamera::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 OpenGLSegmentationCamera::GetViewMatrix() const
{
    return cameraTransform;
}

GLfloat OpenGLSegmentationCamera::GetFOVX() const
{
    return fov.x;
}

GLfloat OpenGLSegmentationCamera::GetFOVY() const
{
    return fov.y;
}

GLfloat OpenGLSegmentationCamera::GetNearClip() const
{
    return range.x;
}

GLfloat OpenGLSegmentationCamera::GetFarClip() const
{
    return range.y;
}

void OpenGLSegmentationCamera::Update()
{
    _needsUpdate = true;
}

bool OpenGLSegmentationCamera::needsUpdate()
{
    if(_needsUpdate)
    {
        _needsUpdate = false;
        return enabled;
    }
    else
        return false;
}

void OpenGLSegmentationCamera::setCamera(Camera* cam, unsigned int index)
{
    camera = cam;

    glGenBuffers(1, &outputPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * sizeof(GLushort), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenBuffers(1, &displayPBO);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, viewportWidth * viewportHeight * 3 * sizeof(GLubyte), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

ViewType OpenGLSegmentationCamera::getType() const
{
    return ViewType::SEGMENTATION_CAMERA;
}

void OpenGLSegmentationCamera::ComputeOutput(std::vector<Renderable>& objects, Ocean* ocean)
{
    OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
    content->SetCurrentView(this);
    content->SetDrawingMode(DrawingMode::RAW);
    
    //Segmentation object id output
    OpenGLState::BindFramebuffer(renderFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 VP = GetProjectionMatrix() * GetViewMatrix();

    segmentationCameraOutputShader->Use();
    segmentationCameraOutputShader->SetUniform("FC", GetLogDepthConstant());
    
    for(size_t i=0; i<objects.size(); ++i)
    {
        if(objects[i].type != RenderableType::SOLID && objects[i].objectId >= 0)
            continue;
        segmentationCameraOutputShader->SetUniform("MVP", VP * objects[i].model);
        segmentationCameraOutputShader->SetUniform("M", objects[i].model);
        segmentationCameraOutputShader->SetUniform("objectId", (GLuint)objects[i].objectId+1);
        content->DrawObject(objects[i].objectId, -1, objects[i].model);
    }

    if(ocean != nullptr && ocean->GetDepth(eye) > 0.f)
    {
        OpenGLOcean* glOcean = ocean->getOpenGLOcean();
        glOcean->DrawParticlesId(this, (GLushort)(UINT16_MAX-1));
    }

    //Flip image
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderSegTex[0]);
    flipShader->Use();
    flipShader->SetUniform("texSource", TEX_POSTPROCESS1);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    
    //Color mapped segmentation display
    OpenGLState::BindFramebuffer(displayFBO);
    OpenGLState::Viewport(0, 0, viewportWidth, viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderSegTex[1]);
    segmentationVisualizeShader->Use();
    segmentationVisualizeShader->SetUniform("texSeg", TEX_POSTPROCESS1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    OpenGLState::BindVertexArray(displayVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    OpenGLState::BindVertexArray(0);
    OpenGLState::BindFramebuffer(0);
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
}

void OpenGLSegmentationCamera::DrawLDR(GLuint destinationFBO, bool updated)
{
    //Check if there is a need to display image on screen
    bool display = true;
    unsigned int dispX, dispY;
    GLfloat dispScale;
    if(camera != nullptr)
        display = camera->getDisplayOnScreen(dispX, dispY, dispScale);
    
    //Draw on screen
    if(display)
    {
        OpenGLContent* content = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent();
        int windowHeight = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowHeight();
        int windowWidth = ((GraphicalSimulationApp*)SimulationApp::getApp())->getWindowWidth();
        OpenGLState::BindFramebuffer(destinationFBO);
        OpenGLState::Viewport(0, 0, windowWidth, windowHeight);
        OpenGLState::DisableCullFace();
        content->DrawTexturedQuad(dispX, dispY+viewportHeight*dispScale, viewportWidth*dispScale, -viewportHeight*dispScale, displaySegTex);
        OpenGLState::EnableCullFace();
        OpenGLState::BindFramebuffer(0);
    }

    //Copy texture to camera buffer
    if(camera != nullptr && updated)
    {
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, renderSegTex[1]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, outputPBO);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::BindTexture(TEX_POSTPROCESS1, GL_TEXTURE_2D, displaySegTex);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, displayPBO);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        OpenGLState::UnbindTexture(TEX_POSTPROCESS1);
        newData = true;
    }
}

///////////////////////// Static /////////////////////////////
void OpenGLSegmentationCamera::Init()
{
    segmentationCameraOutputShader = new GLSLShader("segmentation.frag", "segmentation.vert");
    segmentationCameraOutputShader->AddUniform("MVP", ParameterType::MAT4);
    segmentationCameraOutputShader->AddUniform("M", ParameterType::MAT4);
    segmentationCameraOutputShader->AddUniform("FC", ParameterType::FLOAT);
    segmentationCameraOutputShader->AddUniform("objectId", ParameterType::UINT);
    
    segmentationVisualizeShader = new GLSLShader("segmentationVisualize.frag");
    segmentationVisualizeShader->AddUniform("texSeg", ParameterType::INT);

    flipShader = new GLSLShader("uintVerticalFlip.frag");
    flipShader->AddUniform("texSource", ParameterType::INT);
}

void OpenGLSegmentationCamera::Destroy()
{
    if(segmentationCameraOutputShader != nullptr) delete segmentationCameraOutputShader;
    if(segmentationVisualizeShader != nullptr) delete segmentationVisualizeShader;
    if(flipShader != nullptr) delete flipShader;
}

}
