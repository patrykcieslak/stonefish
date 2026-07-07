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
//  OpenGLConsole.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 02/12/2018.
//  Copyright (c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#include "graphics/OpenGLConsole.h"

#include "utils/SystemUtil.hpp"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/IMGUI.h"
#include "graphics/OpenGLPrinter.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

OpenGLConsole::OpenGLConsole()
{
    windowW_ = 0;
    windowH_ = 0;
    lastTime_ = 0;
    scrollOffset_ = 0.f;
    scrollVelocity_ = 0.f;
    printer_ = NULL;
    logoTexture_ = 0;
    consoleVAO_ = 0;
    texQuadShader_ = NULL;
    lastTime_ = GetTimeInMicroseconds();
}
    
OpenGLConsole::~OpenGLConsole()
{
    if(printer_ != NULL) delete printer_;
    if(logoTexture_ > 0) glDeleteTextures(1, &logoTexture_);
    if(consoleVAO_ > 0) glDeleteVertexArrays(1, &consoleVAO_);
    if(texQuadShader_ != NULL) delete texQuadShader_;
}
    
void OpenGLConsole::Init(int w, int h)
{
    windowW_ = w;
    windowH_ = h;
    OpenGLPrinter::SetWindowSize(windowW_, windowH_);
    
    if(logoTexture_ > 0) //Check if not already initialized
        return;
    //Load logo texture
    logoTexture_ = OpenGLContent::LoadInternalTexture("logo_64.png", false, true);
    
    glGenVertexArrays(1, &consoleVAO_);
    OpenGLState::BindVertexArray(consoleVAO_);
    glEnableVertexAttribArray(0);
    OpenGLState::BindVertexArray(0);
    
    texQuadShader_ = new GLSLShader("texQuad.frag","texQuad.vert");
    texQuadShader_->AddUniform("rect", ParameterType::VEC4);
    texQuadShader_->AddUniform("tex", ParameterType::INT);
    texQuadShader_->AddUniform("color", ParameterType::VEC4);
    
    printer_ = new OpenGLPrinter(GetShaderPath() + std::string(STANDARD_FONT_NAME), STANDARD_FONT_SIZE);
}
    
void OpenGLConsole::Scroll(GLfloat amount)
{
    scrollVelocity_ += 25.f * amount;
}
    
void OpenGLConsole::ResetScroll()
{
    scrollOffset_ = 0.f;
    scrollVelocity_ = 0.f;
}
    
void OpenGLConsole::Render(bool overlay)
{
    if(logoTexture_ == 0)
        return;
    
    int64_t now = GetTimeInMicroseconds();
    GLfloat dt = (lastTime_-now)/1000000.f;
    lastTime_ = now;
        
    if(lines_.size() == 0)
        return;
        
    //Calculate visible lines range
    long int maxVisibleLines = (long int)floorf((GLfloat)windowH_/(GLfloat)(STANDARD_FONT_SIZE + 5)) + 1;
    long int linesCount = lines_.size();
    long int visibleLines = maxVisibleLines;
    long int scrolledLines = 0;
        
    GLfloat logoSize = 64.f;
    GLfloat logoMargin = 10.f;
    glm::vec4 info(1.f, 1.f, 1.f, 1.f);
    glm::vec4 warning(1.f, 1.f, 0.f, 1.f);
    glm::vec4 error(1.f, 0.f, 0.f, 1.f);
    
    if(linesCount < maxVisibleLines) //there is enough space to display all lines
    {
        scrollOffset_ = 0.f;
        scrollVelocity_ = 0.f;
        visibleLines = linesCount;
    }
    else //there is more lines than can appear on screen at once
    {
        scrolledLines = (long int)floorf(-scrollOffset_ /(GLfloat)(STANDARD_FONT_SIZE + 5));
        scrolledLines = scrolledLines < 0 ? 0 : scrolledLines;
        if(linesCount < scrolledLines + visibleLines)
            visibleLines = linesCount - scrolledLines;
        
        if(scrollVelocity_ != 0.f) //velocity damping (momentum effect)
        {
            scrollOffset_ += scrollVelocity_ * dt;
            GLfloat dampingAcc = scrollVelocity_ * 1.f;
            scrollVelocity_ += dampingAcc * dt;
        }
        
        //springy effect
        if(scrollOffset_ > 0.f) //the list is scrolled up too much
        {
            GLfloat newVelocity = scrollVelocity_ - scrollOffset_ * 10.f * dt;
            if(newVelocity > 0.f)
                scrollVelocity_ = scrollOffset_ * 2.f;
            else
                scrollVelocity_ = newVelocity;
        }
        else if(visibleLines < (maxVisibleLines - 1)) //the list is scrolled down too much
        {
            GLfloat newVelocity = scrollVelocity_ + (1.f - (GLfloat)visibleLines/(GLfloat)(maxVisibleLines - 2)) * (GLfloat)windowH_ * 10.f * dt;
            if(newVelocity < 0.f)
                scrollVelocity_ = -((1.f - (GLfloat)visibleLines/(GLfloat)(maxVisibleLines - 1)) * (GLfloat)windowH_) * 2.f;
            else
                scrollVelocity_ = newVelocity;
        }
    }
    
    //Setup viewport and ortho
    if(overlay)
    {
        glScissor(0, 0, windowW_, windowH_);
        OpenGLState::Viewport(0, 0, windowW_, windowH_);
        OpenGLState::DisableDepthTest();
        OpenGLState::DisableCullFace();
        OpenGLState::EnableBlend();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        texQuadShader_->Use();
        texQuadShader_->SetUniform("tex", 0);
        texQuadShader_->SetUniform("color",  glm::vec4(0.3f,0.3f,0.3f,1.f));
        texQuadShader_->SetUniform("rect", glm::vec4(0, 0, 1.f, 1.f));
        
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, ((GraphicalSimulationApp*)SimulationApp::getApp())->getGUI()->getTranslucentTexture());
        OpenGLState::BindVertexArray(consoleVAO_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        texQuadShader_->SetUniform("color",  glm::vec4(1.f,1.f,1.f,1.f));
        texQuadShader_->SetUniform("rect", glm::vec4((windowW_ - logoSize - logoMargin)/(GLfloat)windowW_, 1.f - (logoMargin+logoSize)/(GLfloat)windowH_, logoSize/(GLfloat)windowW_, logoSize/(GLfloat)windowH_));
        
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, logoTexture_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        OpenGLState::UseProgram(0);
        OpenGLState::UnbindTexture(TEX_BASE);
        
        //Text rendering
        for(long int i = scrolledLines; i < scrolledLines + visibleLines; i++)
        {
            ConsoleMessage* msg = &lines_[linesCount-1-i];
            glm::vec4 color;
            switch(msg->type)
            {
                case MessageType::INFO:
                    color = info;
                    break;
                case MessageType::WARNING:
                    color = warning;
                    break;
                case MessageType::ERROR:
                case MessageType::CRITICAL:
                    color = error;
                    break;
            }
            printer_->Print(msg->text.c_str(), color, 10.f, scrollOffset_ + 10.f + i * (STANDARD_FONT_SIZE + 5), STANDARD_FONT_SIZE);
        }
        
        OpenGLState::BindVertexArray(0);
        OpenGLState::DisableBlend();
        OpenGLState::EnableDepthTest();
        OpenGLState::EnableCullFace();
    }
    else //During loading of resources (displaying in second thread -> no VAO sharing)
    {
        glUseProgram(texQuadShader_->getProgramHandle());
        texQuadShader_->SetUniform("tex", 0);
        texQuadShader_->SetUniform("color",  glm::vec4(1.f,1.f,1.f,1.f));
        texQuadShader_->SetUniform("rect", glm::vec4((windowW_ - logoSize - logoMargin)/(GLfloat)windowW_, 1.f - (logoMargin+logoSize)/(GLfloat)windowH_, logoSize/(GLfloat)windowW_, logoSize/(GLfloat)windowH_));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, logoTexture_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        
        //Text rendering
        for(long int i = scrolledLines; i < scrolledLines + visibleLines; i++)
        {
            ConsoleMessage* msg = &lines_[linesCount-1-i];
            glm::vec4 color;
            switch(msg->type)
            {
                case MessageType::INFO:
                    color = info;
                    break;
                case MessageType::WARNING:
                    color = warning;
                    break;
                case MessageType::ERROR:
                case MessageType::CRITICAL:
                    color = error;
                    break;
            }
            printer_->Print(msg->text.c_str(), color, 10.f, scrollOffset_ + 10.f + i * (STANDARD_FONT_SIZE + 5), STANDARD_FONT_SIZE, true);
        }
    }
    
    
}
    
}
