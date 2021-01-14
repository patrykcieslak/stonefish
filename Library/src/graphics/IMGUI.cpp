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
//  IMGUI.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/27/12.
//  Copyright (c) 2012-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/IMGUI.h"

#include "core/SimulationApp.h"
#include "graphics/OpenGLState.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "graphics/OpenGLPrinter.h"
#include "graphics/GLSLShader.h"
#include "sensors/ScalarSensor.h"
#include "sensors/Sample.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

IMGUI::IMGUI(GLint windowWidth, GLint windowHeight, GLfloat hue)
{
    shaders = false;
    mouseX = 0;
    mouseY = 0;
    mouseLeftDown = false;
    mouseRightDown = false;
    clearActive();
    guiVAO = 0;
    translucentFBO = 0;
    translucentTexture[0] = 0;
    translucentTexture[1] = 0;
    downsampleShader = NULL;
    gaussianShader = NULL;
    guiShader[0] = NULL;
    guiShader[1] = NULL;
    plainPrinter = NULL;
    logoTexture = 0;
    guiTexture = 0;
    
    //Set interface colors
    theme[PANEL_COLOR] = glm::vec4(0.98f, 0.98f, 0.98f, 1.f);
    theme[ACTIVE_TEXT_COLOR] = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
    theme[INACTIVE_TEXT_COLOR] = glm::vec4(0.7f, 0.7f, 0.7f, 0.5f);
    theme[ACTIVE_CONTROL_COLOR] = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
    theme[INACTIVE_CONTROL_COLOR] = glm::vec4(0.9f, 0.9f, 0.9f, 0.5f);
    theme[HOT_CONTROL_COLOR] = glm::vec4(Color::HSV(hue, 0.1f, 1.0f).rgb, 1.f);
    theme[PUSHED_CONTROL_COLOR] = glm::vec4(Color::HSV(hue, 0.2f, 0.8f).rgb, 1.f);
    theme[EMPTY_COLOR] = glm::vec4(0.6f, 0.6f, 0.6f, 0.9f);
    theme[FILLED_COLOR] = glm::vec4(Color::HSV(hue, 1.f, 0.8f).rgb, 0.9f);
    theme[PLOT_COLOR] = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    theme[PLOT_TEXT_COLOR] = glm::vec4(0.9f, 0.9f, 0.9f, 0.9f);
    
    //Set size
    Resize(windowWidth, windowHeight);
    
    //Create printers
    plainPrinter = new OpenGLPrinter(GetShaderPath() + std::string(STANDARD_FONT_NAME), STANDARD_FONT_SIZE);
    backgroundMargin = 5.f;
    
    //Load logo texture
    logoTexture = OpenGLContent::LoadInternalTexture("logo_gray_64.png", true);
    //Load corner texture
    guiTexture = OpenGLContent::LoadInternalTexture("gui.png", true);
    
    //Generate VAO
    glGenVertexArrays(1, &guiVAO);
    OpenGLState::BindVertexArray(guiVAO);
    glEnableVertexAttribArray(0);
    OpenGLState::BindVertexArray(0);
    
    //Load translucent shaders
    downsampleShader = new GLSLShader("downsample2x.frag");
    downsampleShader->AddUniform("source", INT);
    downsampleShader->AddUniform("srcViewport", VEC2);
    gaussianShader = new GLSLShader("gaussianBlur.frag", "gaussianBlur.vert");
    gaussianShader->AddUniform("source", INT);
    gaussianShader->AddUniform("texelOffset", VEC2);
    guiShader[0] = new GLSLShader("guiFlat.frag","guiFlat.vert");
    guiShader[0]->AddUniform("color", VEC4);
    guiShader[1] = new GLSLShader("guiTex.frag","guiTex.vert");
    guiShader[1]->AddUniform("tex", INT);
    guiShader[1]->AddUniform("backTex", INT);
    guiShader[1]->AddUniform("color", VEC4);
}

void IMGUI::Resize(GLint windowWidth, GLint windowHeight)
{
    windowW = windowWidth;
    windowH = windowHeight;
    
    //Resize printing area
    OpenGLPrinter::SetWindowSize(windowW, windowH);
    
    //Destroy translucent background textures and framebuffers
    if(translucentTexture[0] != 0) glDeleteTextures(2, translucentTexture);
    if(translucentFBO != 0) glDeleteFramebuffers(1, &translucentFBO);
    
    //Create translucent background resources
    glGenFramebuffers(1, &translucentFBO);
    OpenGLState::BindFramebuffer(translucentFBO);
    
    glGenTextures(1, &translucentTexture[0]);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, translucentTexture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, windowW/4, windowH/4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, translucentTexture[0], 0);
    
    glGenTextures(1, &translucentTexture[1]);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, translucentTexture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, windowW/4, windowH/4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, translucentTexture[1], 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Translucent background FBO initialization failed!");
    
    OpenGLState::BindFramebuffer(0);
}

IMGUI::~IMGUI()
{
    if(plainPrinter != NULL)
        delete plainPrinter;
    if(logoTexture > 0)
        glDeleteTextures(1, &logoTexture);
    if(guiTexture > 0) 
        glDeleteTextures(1, &guiTexture);
    if(downsampleShader != NULL)
        delete downsampleShader;
    if(gaussianShader != NULL)
        delete gaussianShader;
    if(guiShader[0] != NULL)
        delete guiShader[0];
    if(guiShader[1] != NULL)
        delete guiShader[1];
    if(translucentTexture[0] > 0)
        glDeleteTextures(2, translucentTexture);
    if(guiVAO > 0)
        glDeleteVertexArrays(1, &guiVAO);
    if(translucentFBO > 0)
        glDeleteFramebuffers(1, &translucentFBO);
}

Uid IMGUI::getHot()
{
    return hot;
}

Uid IMGUI::getActive()
{
    return active;
}

void IMGUI::setHot(Uid newHot)
{
    hot = newHot;
}

void IMGUI::setActive(Uid newActive)
{
    active = newActive;
}

bool IMGUI::isHot(Uid id)
{
    return (hot.owner == id.owner && hot.item == id.item && hot.index == id.index);
}

bool IMGUI::isActive(Uid id)
{
    return (active.owner == id.owner && active.item == id.item && active.index == id.index);
}

bool IMGUI::isAnyActive()
{
    return (active.owner != -1);
}

void IMGUI::clearActive()
{
    active.owner = -1;
}

void IMGUI::clearHot()
{
    hot.owner = -1;
}

int IMGUI::getMouseX()
{
    return mouseX;
}

int IMGUI::getMouseY()
{
    return mouseY;
}

int IMGUI::getWindowHeight()
{
    return windowH;
}

int IMGUI::getWindowWidth()
{
    return windowW;
}

GLuint IMGUI::getTranslucentTexture()
{
    return translucentTexture[0];
}

void IMGUI::GenerateBackground()
{
    OpenGLState::BindFramebuffer(translucentFBO);
    OpenGLState::Viewport(0, 0, windowW/4, windowH/4);
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getScreenTexture());
    downsampleShader->Use();
    downsampleShader->SetUniform("source", TEX_BASE);
    downsampleShader->SetUniform("srcViewport", glm::vec2((GLfloat)windowW, (GLfloat)windowH));
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    
    gaussianShader->Use();
    gaussianShader->SetUniform("source", TEX_BASE);
    for(int i=0; i<3; ++i)
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, translucentTexture[0]);
        gaussianShader->SetUniform("texelOffset", glm::vec2(4.f/(GLfloat)windowW, 0.f));
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
        
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, translucentTexture[1]);
        gaussianShader->SetUniform("texelOffset", glm::vec2(0.f, 4.f/(GLfloat)windowH));
        ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawSAQ();
    }
    OpenGLState::UseProgram(0);
    OpenGLState::UnbindTexture(TEX_BASE);
    OpenGLState::BindFramebuffer(0);
}

void IMGUI::Begin()
{
    clearHot();
    
    OpenGLState::DisableDepthTest();
    OpenGLState::DisableCullFace();
    OpenGLState::EnableBlend();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glScissor(0, 0, windowW, windowH);
    OpenGLState::Viewport(0, 0, windowW, windowH);
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->SetViewportSize(windowW, windowH);
    OpenGLState::BindVertexArray(guiVAO);
}

void IMGUI::End()
{
    OpenGLState::BindVertexArray(0);
    
    //draw logo on top
    GLfloat logoSize = 64.f;
    GLfloat logoMargin = 10.f;
    ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->DrawTexturedQuad(windowW - logoSize - logoMargin, logoMargin, logoSize, logoSize, logoTexture, glm::vec4(1.f,1.f,1.f,0.2f));
   
    OpenGLState::EnableDepthTest();
    OpenGLState::EnableCullFace();
    OpenGLState::DisableBlend();
}

void IMGUI::DrawPlainText(GLfloat x, GLfloat y, glm::vec4 color, const std::string& text, GLfloat scale)
{
    plainPrinter->Print(text, color, x, windowH - y - STANDARD_FONT_BASELINE * scale, STANDARD_FONT_SIZE * scale);
}

GLfloat IMGUI::PlainTextLength(const std::string& text)
{
    return plainPrinter->TextLength(text);
}

glm::vec2 IMGUI::PlainTextDimensions(const std::string& text)
{
    return plainPrinter->TextDimensions(text);
}

bool IMGUI::MouseInRect(int x, int y, int w, int h)
{
    return (mouseX >= x && mouseX <= (x+w) && mouseY >= y && mouseY <= (y+h));
}

bool IMGUI::MouseIsDown(bool leftButton)
{
    if(leftButton)
        return mouseLeftDown;
    else
        return mouseRightDown;
}

void IMGUI::MouseDown(int x, int y, bool leftButton)
{
    mouseX = x;
    mouseY = y;
    
    if(leftButton)
        mouseLeftDown = true;
    else
        mouseRightDown = true;
}

void IMGUI::MouseUp(int x, int y, bool leftButton)
{
    mouseX = x;
    mouseY = y;
    
    if(leftButton)
        mouseLeftDown = false;
    else
        mouseRightDown = false;
}

void IMGUI::MouseMove(int x, int y)
{
    mouseX = x;
    mouseY = y;
}

void IMGUI::KeyDown(SDL_Keycode key)
{
}

void IMGUI::KeyUp(SDL_Keycode key)
{
}

void IMGUI::DrawArrow(GLfloat x, GLfloat y, GLfloat h, bool up, glm::vec4 color)
{
    y = windowH - y;
    
    GLfloat hx = h/windowW * 2.f;
    GLfloat hy = h/windowH * 2.f;
    x = x/windowW * 2.f - 1.f;
    y = y/windowH * 2.f - 1.f;
    
    GLfloat triData[3][2];
    if(up)
    {
        triData[1][0] = x-hx/2.f;
        triData[1][1] = y-hy/2.f;
        triData[0][0] = x+hx/2.f;
        triData[0][1] = y-hy/2.f;
        triData[2][0] = x;
        triData[2][1] = y+hy/2.f;
    }
    else
    {
        triData[1][0] = x-hx/2.f;
        triData[1][1] = y+hy/2.f;
        triData[0][0] = x+hx/2.f;
        triData[0][1] = y+hy/2.f;
        triData[2][0] = x;
        triData[2][1] = y-hy/2.f;
    }

    GLuint vbo;
    glGenBuffers(1, &vbo);
        
    guiShader[0]->Use();
    guiShader[0]->SetUniform("color", color);
        
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triData), triData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    
    OpenGLState::UseProgram(0);
    glDeleteBuffers(1, &vbo);
}

void IMGUI::DrawRect(GLfloat x, GLfloat y, GLfloat w, GLfloat h, glm::vec4 color)
{
    y = windowH - y;
    
    w = w/windowW * 2.f;
    h = h/windowH * 2.f;
    x = x/windowW * 2.f - 1.f;
    y = y/windowH * 2.f - 1.f;
    
    GLfloat rectData[4][2]= {{x,     y},
                             {x,   y-h},
                             {x+w,   y},
                             {x+w, y-h}};
                             
    GLuint vbo;
    glGenBuffers(1, &vbo);
        
    guiShader[0]->Use();
    guiShader[0]->SetUniform("color", color);
        
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectData), rectData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    OpenGLState::UseProgram(0);
    glDeleteBuffers(1, &vbo);
}

void IMGUI::DrawRoundedRect(GLfloat x, GLfloat y, GLfloat w, GLfloat h, glm::vec4 color)
{
    w = w < 4.f*CORNER_RADIUS ? 4.f*CORNER_RADIUS : w;
    h = h < 4.f*CORNER_RADIUS ? 4.f*CORNER_RADIUS : h;
    y = windowH - y;
    
    glm::vec2 cs(CORNER_RADIUS/windowW*2.f, CORNER_RADIUS/windowH*2.f);
    w = w/windowW * 2.f;
    h = h/windowH * 2.f;
    x = x/windowW * 2.f - 1.f;
    y = y/windowH * 2.f - 1.f;
    
    GLfloat rectData[24][4]= {{x,             y, 0.f, 0.f},
                              {x,        y-cs.y, 0.f, 0.333f},
                              {x+cs.x,        y, 0.333f, 0.f},
                              {x+cs.x,   y-cs.y, 0.333f, 0.333f},
                              {x+w-cs.x,      y, 0.666f, 0.f},
                              {x+w-cs.x, y-cs.y, 0.666f, 0.333f},
                              {x+w,           y, 1.f, 0.f},
                              {x+w,      y-cs.y, 1.f, 0.333f},
                              
                              {x,          y-cs.y, 0.f, 0.333f},
                              {x,        y-h+cs.y, 0.f, 0.666f},
                              {x+cs.x,     y-cs.y, 0.333f, 0.333f},
                              {x+cs.x,   y-h+cs.y, 0.333f, 0.666f},
                              {x+w-cs.x,   y-cs.y, 0.666f, 0.333f},
                              {x+w-cs.x, y-h+cs.y, 0.666f, 0.666f},
                              {x+w,        y-cs.y, 1.f, 0.333f},
                              {x+w,      y-h+cs.y, 1.f, 0.666f},
                              
                              {x,        y-h+cs.y, 0.f, 0.666f},
                              {x,             y-h, 0.f, 1.f},
                              {x+cs.x,   y-h+cs.y, 0.333f, 0.666f},
                              {x+cs.x,        y-h, 0.333f, 1.f},
                              {x+w-cs.x, y-h+cs.y, 0.666f, 0.666f},
                              {x+w-cs.x,      y-h, 0.666f, 1.f},
                              {x+w,      y-h+cs.y, 1.f, 0.666f},
                              {x+w,           y-h, 1.f, 1.f}};
    
    //Get translucent texture
    OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, guiTexture);
    OpenGLState::BindTexture(TEX_GUI1, GL_TEXTURE_2D, getTranslucentTexture());
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
        
    guiShader[1]->Use();
    guiShader[1]->SetUniform("tex", 0);
    guiShader[1]->SetUniform("backTex", 1);
    guiShader[1]->SetUniform("color", color);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectData), rectData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
    glDrawArrays(GL_TRIANGLE_STRIP, 8, 8);
    glDrawArrays(GL_TRIANGLE_STRIP, 16, 8);
    
    OpenGLState::UseProgram(0);
    glDeleteBuffers(1, &vbo);
}

void IMGUI::DoPanel(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
    DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
}

void IMGUI::DoLabel(GLfloat x, GLfloat y, const std::string& text, glm::vec4 color, GLfloat scale)
{
    if(color.r < 0.f)
        DrawPlainText(x, y, theme[ACTIVE_TEXT_COLOR], text, scale);
    else
        DrawPlainText(x, y, color, text, scale);
}

bool IMGUI::DoButton(Uid id, GLfloat x, GLfloat y, GLfloat w, GLfloat h, const std::string& title)
{
    bool result = false;
    
    if(MouseInRect(x, y, w, h))
        setHot(id);
    
    if(isActive(id))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(id))
                result = true;
            clearActive();
        }
    }
    else if(isHot(id))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(id);
    }
    
    //drawing
    if(isActive(id))
        DrawRoundedRect(x, y, w, h, theme[PUSHED_CONTROL_COLOR]);
    else if(isHot(id))
        DrawRoundedRect(x, y, w, h, theme[HOT_CONTROL_COLOR]);
    else
        DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
        
    glm::vec2 textDim = PlainTextDimensions(title);
    DrawPlainText(x + floorf((w - textDim.x)/2.f), y + floorf((h - textDim.y)/2.f), theme[ACTIVE_TEXT_COLOR], title);
    
    return result;
}

Scalar IMGUI::DoSlider(Uid id, GLfloat x, GLfloat y, GLfloat w, Scalar min, Scalar max, Scalar value, const std::string& title, unsigned int decimalPlaces)
{
    //Check and correct dimensions
    w = w < 8*backgroundMargin ? 8.f*backgroundMargin : w;
    
    GLfloat railW = w - 4.f*backgroundMargin;
    GLfloat railH = 5.f;
    GLfloat sliderW = 5.f;
    GLfloat sliderH = 20.f;
    GLfloat h = sliderH + 2.f * backgroundMargin + 5.f + STANDARD_FONT_SIZE;
    
    //Check mouse position
    Scalar result = value;
    GLfloat sliderPosition = (value-min)/(max-min);
    
    if(MouseInRect(x + backgroundMargin*2.f + sliderPosition * railW - sliderW/2.f, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f, sliderW, sliderH))
        setHot(id);
    
    if(isActive(id))
    {
        GLfloat mouseX = getMouseX();
        if(mouseX <= x + backgroundMargin*2.f)
            sliderPosition = 0;
        else if(mouseX >= x + backgroundMargin*2.f + railW)
            sliderPosition = 1.f;
        else
            sliderPosition = (mouseX - x - backgroundMargin*2.f)/railW;
        
        result = min + sliderPosition * (max - min);
        
        if(!MouseIsDown(true)) //mouse went up
            clearActive();
    }
    else if(isHot(id))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(id);
    }
    
    //Drawing
    //Background and text
    DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
    DrawPlainText(x + backgroundMargin, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], title);
    
    char buffer[16];
    std::string format = "%1." + std::to_string(decimalPlaces) + "lf";
    sprintf(buffer, format.c_str(), result);
    glm::vec2 textDim = PlainTextDimensions(buffer);
    DrawPlainText(x + railW + 3.f*backgroundMargin - textDim.x, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], buffer);
    
    //Bar
    DrawRect(x+backgroundMargin*2.f, y+backgroundMargin+STANDARD_FONT_SIZE+5.f+sliderH/2.f-railH/2.f, sliderPosition * railW, railH, theme[FILLED_COLOR]);
    DrawRect(x+backgroundMargin*2.f + sliderPosition * railW, y + backgroundMargin+STANDARD_FONT_SIZE+5.f+sliderH/2.f-railH/2.f, railW - sliderPosition*railW, railH, theme[EMPTY_COLOR]);
    
    //Slider
    if(isActive(id))
        DrawRect(x+backgroundMargin*2.f + sliderPosition * railW - sliderW/2.f, y+backgroundMargin+STANDARD_FONT_SIZE+5.f, sliderW, sliderH, theme[PUSHED_CONTROL_COLOR]);
    else if(isHot(id))
        DrawRect(x+backgroundMargin*2.f + sliderPosition * railW - sliderW/2.f, y+backgroundMargin+STANDARD_FONT_SIZE+5.f, sliderW, sliderH, theme[HOT_CONTROL_COLOR]);
    else
        DrawRect(x+backgroundMargin*2.f + sliderPosition * railW - sliderW/2.f, y+backgroundMargin+STANDARD_FONT_SIZE+5.f, sliderW, sliderH, theme[ACTIVE_CONTROL_COLOR]);
    
    return result;
}

void IMGUI::DoProgressBar(GLfloat x, GLfloat y, GLfloat w, Scalar progress, const std::string& title)
{
    //Check and correct dimensions
    w = w < 8*backgroundMargin ? 8.f*backgroundMargin : w;
    GLfloat barW = w - 4.f*backgroundMargin;
    GLfloat barH = 5.f;
    GLfloat h =  barH + 2.f * backgroundMargin + 5.f + STANDARD_FONT_SIZE;
    
    //Check and correct progress value
    progress = progress < Scalar(0.) ? Scalar(0.) : (progress > Scalar(1.) ? Scalar(1.) : progress);
    
    //Draw background and title
    DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
    DrawPlainText(x + backgroundMargin, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], title);
    
    char buffer[16];
    sprintf(buffer, "%1.1lf%%", progress * 100.0);
    GLfloat len = PlainTextLength(buffer);
    DrawPlainText(x + 3.f*backgroundMargin + barW - len, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], buffer);
    
    //Draw bar
    DrawRect(x+backgroundMargin*2.f, y+backgroundMargin+STANDARD_FONT_SIZE+5.f-barH/2.f, progress * barW, barH, theme[FILLED_COLOR]);
    DrawRect(x+backgroundMargin*2.f + progress * barW, y + backgroundMargin+STANDARD_FONT_SIZE+5.f-barH/2.f, barW - progress*barW, barH, theme[EMPTY_COLOR]);
}

bool IMGUI::DoCheckBox(Uid id, GLfloat x, GLfloat y, GLfloat w, bool value, const std::string& title)
{
    bool result = value;
    GLfloat size = 14.f;
    w = w < size + 2.f*backgroundMargin ? size + 2.f*backgroundMargin : w;
    GLfloat h = size + 2.f * backgroundMargin;
    
    if(MouseInRect(x + backgroundMargin, y + backgroundMargin, size, size))
        setHot(id);
    
    if(isActive(id))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(id))
                result = !result;
                
            clearActive();
        }
    }
    else if(isHot(id))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(id);
    }
    
    //drawing
    glm::vec2 textDim = PlainTextDimensions(title);
    DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
    DrawPlainText(x + size + backgroundMargin + 5.f, y + backgroundMargin + size/2.f - textDim.y/2.f, theme[ACTIVE_TEXT_COLOR], title);
    
    if(isActive(id))
        DrawRect(x+backgroundMargin, y+backgroundMargin, size, size, theme[PUSHED_CONTROL_COLOR]);
    else if(isHot(id))
        DrawRect(x+backgroundMargin, y+backgroundMargin, size, size, theme[HOT_CONTROL_COLOR]);
    else
        DrawRect(x+backgroundMargin, y+backgroundMargin, size, size, theme[ACTIVE_CONTROL_COLOR]);
    
    if(result)
        DrawRect(x+backgroundMargin + 0.2f*size, y+backgroundMargin+0.2f*size, 0.6*size, 0.6*size, theme[FILLED_COLOR]);
    
    return result;
}

unsigned int IMGUI::DoComboBox(Uid id, GLfloat x, GLfloat y, GLfloat w, const std::vector<std::string>& options, unsigned int value, const std::string& title)
{
    value = value >= options.size() ? 0 : value;
    unsigned int result = value;
    GLfloat size = 14.f;
    
    //drawing
    glm::vec2 maxTextDim(0.f,(GLfloat)STANDARD_FONT_SIZE);
    
    for(size_t i=0; i<options.size(); ++i)
    {
        glm::vec2 textDim = PlainTextDimensions(options[i]);
        if(textDim.x > maxTextDim.x) maxTextDim.x = textDim.x;
        if(textDim.y > maxTextDim.y) maxTextDim.y = textDim.y;
    }
    
    GLfloat comboW = maxTextDim.x + 10.f;
    GLfloat comboH = maxTextDim.y + 10.f;
    GLfloat minW = comboW + 2.f*backgroundMargin + 2.f*size + 5.f;
    GLfloat h = comboH + 2.f * backgroundMargin + 5.f + STANDARD_FONT_SIZE;
    
    if(w <= minW)
        w = minW;
    else
        comboW = w - (minW - comboW);
    
    //Background and title
    DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
    DrawPlainText(x + backgroundMargin, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], title);
    
    //Combo box
    if(options.size() == 0)
    {
        DrawRect(x + backgroundMargin, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f, comboW, comboH, theme[EMPTY_COLOR]);
    }
    else
    {
        DrawRect(x + backgroundMargin, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f, comboW, comboH, theme[FILLED_COLOR]);
        DrawPlainText(x + backgroundMargin + 5.f, y + backgroundMargin + STANDARD_FONT_SIZE + 10.f, theme[ACTIVE_TEXT_COLOR], options[value]);
    }
    
    //Buttons
    if(options.size() <= 1)
    {
        DrawArrow(x + backgroundMargin + comboW + 5.f + size/2.f, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f + comboH/2.f, size, false, theme[INACTIVE_CONTROL_COLOR]);
        DrawArrow(x + backgroundMargin + comboW + 5.f + size/2.f + size, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f + comboH/2.f, size, true, theme[INACTIVE_CONTROL_COLOR]);
        return 0;
    }
    
    if(MouseInRect(x + backgroundMargin + comboW + 5.f, y + backgroundMargin + STANDARD_FONT_SIZE + 10.f, size, size))
    {
        id.index = 0;
        setHot(id);
    }
    else if(MouseInRect(x + backgroundMargin + comboW + 5.f + size, y + backgroundMargin + STANDARD_FONT_SIZE + 10.f, size, size))
    {
        id.index = 1;
        setHot(id);
    }
    
    int change = 0;
    
    //Arrow down
    id.index = 0;
    
    if(isActive(id))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(id))
                change = 1;
            clearActive();
        }
    }
    else if(isHot(id))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(id);
    }
    
    if(isActive(id))
        DrawArrow(x + backgroundMargin + comboW + 5.f + size/2.f, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f + comboH/2.f, size, false, theme[PUSHED_CONTROL_COLOR]);
    else if(isHot(id))
        DrawArrow(x + backgroundMargin + comboW + 5.f + size/2.f, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f + comboH/2.f, size, false, theme[HOT_CONTROL_COLOR]);  
    else
        DrawArrow(x + backgroundMargin + comboW + 5.f + size/2.f, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f + comboH/2.f, size, false, theme[ACTIVE_CONTROL_COLOR]);
    
    //Arrow up
    id.index = 1;
    
    if(isActive(id))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(id))
                change = -1;
            clearActive();
        }
    }
    else if(isHot(id))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(id);
    }
    
    if(isActive(id))
        DrawArrow(x + backgroundMargin + comboW + 5.f + size/2.f + size, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f + comboH/2.f, size, true, theme[PUSHED_CONTROL_COLOR]);
    else if(isHot(id))
        DrawArrow(x + backgroundMargin + comboW + 5.f + size/2.f + size, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f + comboH/2.f, size, true, theme[HOT_CONTROL_COLOR]);  
    else
        DrawArrow(x + backgroundMargin + comboW + 5.f + size/2.f + size, y + backgroundMargin + STANDARD_FONT_SIZE + 5.f + comboH/2.f, size, true, theme[ACTIVE_CONTROL_COLOR]);
    
    if(options.size() > 1)
    {
        if(change == -1 && result > 0)
            --result;
        else if(change == 1 && result < options.size()-1)
            ++result;
    }
        
    return result;
}

bool IMGUI::DoTimePlot(Uid id, GLfloat x, GLfloat y, GLfloat w, GLfloat h, ScalarSensor* sens, std::vector<unsigned short>& dims, const std::string& title, Scalar fixedRange[2])
{
    bool result = false;
    GLfloat pltW = w/windowW * 2.f;
    GLfloat pltH = h/windowH * 2.f;
    GLfloat pltX = x/windowW * 2.f - 1.f;
    GLfloat pltY = (windowH-y)/windowH * 2.f - 1.f;
    GLfloat pltMargin = 10.f/windowH*2.f; 
    
    if(MouseInRect(x, y, w, h))
        setHot(id);
    
    if(isActive(id))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(id))
                result = true;
            clearActive();
        }
    }
    else if(isHot(id))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(id);
    }
    
    //Drawing
    DrawRoundedRect(x, y, w, h, theme[PLOT_COLOR]);
    
    //data
    const std::vector<Sample>* data = sens->getHistory();
    
    if(data->size() > 1)
    {
        GLfloat minValue;
        GLfloat maxValue;
        
        if(fixedRange != NULL)
        {
            minValue = fixedRange[0];
            maxValue = fixedRange[1];
        }
        else //Autoscale
        {
            minValue = 10e12;
            maxValue = -10e12;
        
            for(size_t i = 0; i < data->size(); ++i)
            {
                for(size_t n = 0; n < dims.size(); ++n)
                {
                    GLfloat value = (GLfloat)((*data)[i].getValue(dims[n]));
                    if(value > maxValue)
                        maxValue = value;
                    if(value < minValue)
                        minValue = value;
                }
            }
        
            if(maxValue == minValue) //secure division by zero
            {
                maxValue += 0.1f * maxValue;
                minValue -= 0.1f * minValue;
            }
        }
        
        GLfloat dy = (pltH-2.f*pltMargin)/(maxValue-minValue);
        
        //autostretch
        GLfloat dt = pltW/(GLfloat)(data->size()-1);
    
        //drawing
        for(size_t n = 0; n < dims.size(); ++n)
        {
            //set color
            glm::vec4 color;
            
            if(dims.size() > 1)
                color = glm::vec4(Color::HSV(n/(GLfloat)(dims.size()-1)*0.5, 1.f, 1.f).rgb, 1.f);
            else
                color = theme[FILLED_COLOR];
            
            //draw graph
            std::vector<glm::vec2> points;
            for(size_t i = 0;  i < data->size(); ++i)
            {
                GLfloat value = (GLfloat)((*data)[i].getValue(dims[n]));
                points.push_back(glm::vec2(pltX + dt*i, pltY - pltH + pltMargin + (value-minValue) * dy));
            }
            
            GLuint vbo;
            glGenBuffers(1, &vbo);
        
            guiShader[0]->Use();
            guiShader[0]->SetUniform("color", color);
        
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*points.size(), &points[0].x, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (void*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)points.size());
            
            OpenGLState::UseProgram(0);
            glDeleteBuffers(1, &vbo);
            
            //legend
            DrawRect(x - 10.f, y+n*10.f+5.f, 10.f, 10.f, color);
        }
        
        //Grid
        if(minValue < 0.f && maxValue > 0.f)
        {
            GLfloat axisData[2][2] = {{pltX, pltY - pltH + pltMargin - minValue * dy},
                                      {pltX + pltW, pltY - pltH + pltMargin - minValue * dy}};
            
            GLuint vbo;
            glGenBuffers(1, &vbo);
        
            guiShader[0]->Use();
            guiShader[0]->SetUniform("color", theme[PLOT_TEXT_COLOR]);
        
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(axisData), axisData, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (void*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        
            glDrawArrays(GL_LINES, 0, 2);
            
            OpenGLState::UseProgram(0);
            glDeleteBuffers(1, &vbo);
        }
        
        //Check if mouse cursor above legend item and display signal info
        if(MouseInRect(x - 10.f, y + 5.f, 10.f, dims.size() * 10.f)) //mouse above legend
        {
            long selectedDim = (long)floorf((mouseY - y - 5.f)/10.f);
            if(selectedDim < 0)
                selectedDim = 0;
            if(selectedDim > (int)dims.size()-1)
                selectedDim = dims.size()-1;
            
            char buffer[64];
            sprintf(buffer, "%1.6f", sens->getLastSample().getValue(dims[selectedDim]));
            DrawPlainText(x + backgroundMargin, y + backgroundMargin, theme[PLOT_TEXT_COLOR], buffer);
        }
    }
    
    delete data;
        
    //title
    glm::vec2 titleDim = PlainTextDimensions(title);
    DrawPlainText(x + floorf((w - titleDim.x) / 2.f), y + backgroundMargin, theme[PLOT_TEXT_COLOR], title);
    
    return result;
}

bool IMGUI::DoXYPlot(Uid id, GLfloat x, GLfloat y, GLfloat w, GLfloat h, ScalarSensor* sensX, unsigned short dimX, ScalarSensor* sensY, unsigned short dimY, const std::string& title)
{
    bool result = false;
    GLfloat pltW = w/windowW * 2.f;
    GLfloat pltH = h/windowH * 2.f;
    GLfloat pltX = x/windowW * 2.f - 1.f;
    GLfloat pltY = (windowH-y)/windowH * 2.f - 1.f;
    
    if(MouseInRect(x, y, w, h))
        setHot(id);
    
    if(isActive(id))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(id))
                result = true;
            clearActive();
        }
    }
    else if(isHot(id))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(id);
    }
    
    //drawing
    //background
    DrawRoundedRect(x, y, w, h, theme[PLOT_COLOR]);
    
    //data
    const std::vector<Sample>* dataX = sensX->getHistory();
    const std::vector<Sample>* dataY = sensY->getHistory();
    
    if((dataX->size() > 1) && (dataY->size() > 1))
    {
        //common sample count
        unsigned long dataCount = dataX->size();
        if(dataY->size() < dataCount)
            dataCount = dataY->size();
        
        //autoscale X axis
        GLfloat minValueX = 10e12;
        GLfloat maxValueX = -10e12;
        
        for(size_t i = 0; i < dataCount; ++i)
        {
            GLfloat value = (GLfloat)((*dataX)[i].getValue(dimX));
            if(value > maxValueX)
                maxValueX = value;
            if(value < minValueX)
                minValueX = value;
        }
        
        if(maxValueX == minValueX) //secure division by zero
        {
            maxValueX += 0.1f * maxValueX;
            minValueX -= 0.1f * minValueX;
        }
        
        GLfloat dx = pltW/(maxValueX - minValueX);
        
        //autoscale Y axis
        GLfloat minValueY = 10e12;
        GLfloat maxValueY = -10e12;
        
        for(size_t i = 0; i < dataCount; ++i)
        {
            GLfloat value = (GLfloat)((*dataY)[i].getValue(dimY));
            if(value > maxValueY)
                maxValueY = value;
            if(value < minValueY)
                minValueY = value;
        }
        
        if(maxValueY == minValueY) //secure division by zero
        {
            maxValueY += 0.1f * maxValueY;
            minValueY -= 0.1f * minValueY;
        }
        
        GLfloat dy = pltH/(maxValueY - minValueY);
        
        //draw graph
        std::vector<glm::vec2> points;
        
        for(size_t i = 0;  i < dataCount; ++i)
        {
            GLfloat valueX = (GLfloat)((*dataX)[i].getValue(dimX));
            GLfloat valueY = (GLfloat)((*dataY)[i].getValue(dimY));
            points.push_back(glm::vec2(pltX + (valueX - minValueX) * dx, pltY - pltH + (valueY - minValueY) * dy));
        }
        
        GLuint vbo;
        glGenBuffers(1, &vbo);
        
        guiShader[0]->Use();
        guiShader[0]->SetUniform("color", theme[FILLED_COLOR]);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*points.size(), &points[0].x, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)points.size());
        
        OpenGLState::UseProgram(0);
        glDeleteBuffers(1, &vbo);
        
        //draw axes
        std::vector<glm::vec2> axes;
        
        if(minValueX * maxValueX < 0.f)
        {
            axes.push_back(glm::vec2(pltX - minValueX * dx, pltY));
            axes.push_back(glm::vec2(pltX - minValueX * dx, pltY - pltH));
        }
        
        if(minValueY * maxValueY < 0.f)
        {
            axes.push_back(glm::vec2(pltX, pltY - pltH - minValueY * dy));
            axes.push_back(glm::vec2(pltX + pltW, pltY - pltH - minValueY * dy));
        }
        
        if(axes.size() > 0)
        {
            GLuint vbo;
            glGenBuffers(1, &vbo);
        
            guiShader[0]->Use();
            guiShader[0]->SetUniform("color", theme[PLOT_TEXT_COLOR]);
        
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*axes.size(), &axes[0].x, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (void*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        
            glDrawArrays(GL_LINES, 0, (GLsizei)axes.size());
            
            OpenGLState::UseProgram(0);
            glDeleteBuffers(1, &vbo);
        }
    }
    
    delete dataX;
    delete dataY;
    
    //title
    glm::vec2 titleDim = PlainTextDimensions(title);
    DrawPlainText(x + floorf((w - titleDim.x) / 2.f), y + backgroundMargin, theme[PLOT_TEXT_COLOR], title);
    
    return result;
}

}
