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
    windowW = 0;
    windowH = 0;
    lastTime = 0;
    scrollOffset = 0.f;
    scrollVelocity = 0.f;
    printer = NULL;
    logoTexture = 0;
    consoleVAO = 0;
    texQuadShader = NULL;
    lastTime = GetTimeInMicroseconds();
}
    
OpenGLConsole::~OpenGLConsole()
{
    if(printer != NULL) delete printer;
    if(logoTexture > 0) glDeleteTextures(1, &logoTexture);
    if(consoleVAO > 0) glDeleteVertexArrays(1, &consoleVAO);
    if(texQuadShader != NULL) delete texQuadShader;
}
    
void OpenGLConsole::Init(int w, int h)
{
    windowW = w;
    windowH = h;
    OpenGLPrinter::SetWindowSize(windowW, windowH);
    
    if(logoTexture > 0) //Check if not already initialized
        return;
    //Load logo texture
    logoTexture = OpenGLContent::LoadInternalTexture("logo_64.png", true);
    
    glGenVertexArrays(1, &consoleVAO);
    OpenGLState::BindVertexArray(consoleVAO);
    glEnableVertexAttribArray(0);
    OpenGLState::BindVertexArray(0);
    
    texQuadShader = new GLSLShader("texQuad.frag","texQuad.vert");
    texQuadShader->AddUniform("rect", ParameterType::VEC4);
    texQuadShader->AddUniform("tex", ParameterType::INT);
    texQuadShader->AddUniform("color", ParameterType::VEC4);
    
    printer = new OpenGLPrinter(GetShaderPath() + std::string(STANDARD_FONT_NAME), STANDARD_FONT_SIZE);
}
    
void OpenGLConsole::Scroll(GLfloat amount)
{
    scrollVelocity += 25.f * amount;
}
    
void OpenGLConsole::ResetScroll()
{
    scrollOffset = 0.f;
    scrollVelocity = 0.f;
}
    
void OpenGLConsole::Render(bool overlay)
{
    if(logoTexture == 0)
        return;
    
    int64_t now = GetTimeInMicroseconds();
    GLfloat dt = (lastTime-now)/1000000.f;
    lastTime = now;
        
    if(lines.size() == 0)
        return;
        
    //Calculate visible lines range
    long int maxVisibleLines = (long int)floorf((GLfloat)windowH/(GLfloat)(STANDARD_FONT_SIZE + 5)) + 1;
    long int linesCount = lines.size();
    long int visibleLines = maxVisibleLines;
    long int scrolledLines = 0;
        
    GLfloat logoSize = 64.f;
    GLfloat logoMargin = 10.f;
    glm::vec4 info(1.f, 1.f, 1.f, 1.f);
    glm::vec4 warning(1.f, 1.f, 0.f, 1.f);
    glm::vec4 error(1.f, 0.f, 0.f, 1.f);
    
    if(linesCount < maxVisibleLines) //there is enough space to display all lines
    {
        scrollOffset = 0.f;
        scrollVelocity = 0.f;
        visibleLines = linesCount;
    }
    else //there is more lines than can appear on screen at once
    {
        scrolledLines = (long int)floorf(-scrollOffset /(GLfloat)(STANDARD_FONT_SIZE + 5));
        scrolledLines = scrolledLines < 0 ? 0 : scrolledLines;
        if(linesCount < scrolledLines + visibleLines)
            visibleLines = linesCount - scrolledLines;
        
        if(scrollVelocity != 0.f) //velocity damping (momentum effect)
        {
            scrollOffset += scrollVelocity * dt;
            GLfloat dampingAcc = scrollVelocity * 1.f;
            scrollVelocity += dampingAcc * dt;
        }
        
        //springy effect
        if(scrollOffset > 0.f) //the list is scrolled up too much
        {
            GLfloat newVelocity = scrollVelocity - scrollOffset * 10.f * dt;
            if(newVelocity > 0.f)
                scrollVelocity = scrollOffset * 2.f;
            else
                scrollVelocity = newVelocity;
        }
        else if(visibleLines < (maxVisibleLines - 1)) //the list is scrolled down too much
        {
            GLfloat newVelocity = scrollVelocity + (1.f - (GLfloat)visibleLines/(GLfloat)(maxVisibleLines - 2)) * (GLfloat)windowH * 10.f * dt;
            if(newVelocity < 0.f)
                scrollVelocity = -((1.f - (GLfloat)visibleLines/(GLfloat)(maxVisibleLines - 1)) * (GLfloat)windowH) * 2.f;
            else
                scrollVelocity = newVelocity;
        }
    }
    
    //Setup viewport and ortho
    if(overlay)
    {
        glScissor(0, 0, windowW, windowH);
        OpenGLState::Viewport(0, 0, windowW, windowH);
        OpenGLState::DisableDepthTest();
        OpenGLState::DisableCullFace();
        OpenGLState::EnableBlend();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        texQuadShader->Use();
        texQuadShader->SetUniform("tex", 0);
        texQuadShader->SetUniform("color",  glm::vec4(0.3f,0.3f,0.3f,1.f));
        texQuadShader->SetUniform("rect", glm::vec4(0, 0, 1.f, 1.f));
        
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, ((GraphicalSimulationApp*)SimulationApp::getApp())->getGUI()->getTranslucentTexture());
        OpenGLState::BindVertexArray(consoleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        texQuadShader->SetUniform("color",  glm::vec4(1.f,1.f,1.f,1.f));
        texQuadShader->SetUniform("rect", glm::vec4((windowW - logoSize - logoMargin)/(GLfloat)windowW, 1.f - (logoMargin+logoSize)/(GLfloat)windowH, logoSize/(GLfloat)windowW, logoSize/(GLfloat)windowH));
        
        OpenGLState::BindTexture(TEX_BASE, GL_TEXTURE_2D, logoTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        OpenGLState::UseProgram(0);
        OpenGLState::UnbindTexture(TEX_BASE);
        
        //Text rendering
        for(long int i = scrolledLines; i < scrolledLines + visibleLines; i++)
        {
            ConsoleMessage* msg = &lines[linesCount-1-i];
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
            printer->Print(msg->text.c_str(), color, 10.f, scrollOffset + 10.f + i * (STANDARD_FONT_SIZE + 5), STANDARD_FONT_SIZE);
        }
        
        OpenGLState::BindVertexArray(0);
        OpenGLState::DisableBlend();
        OpenGLState::EnableDepthTest();
        OpenGLState::EnableCullFace();
    }
    else //During loading of resources (displaying in second thread -> no VAO sharing)
    {
        glUseProgram(texQuadShader->getProgramHandle());
        texQuadShader->SetUniform("tex", 0);
        texQuadShader->SetUniform("color",  glm::vec4(1.f,1.f,1.f,1.f));
        texQuadShader->SetUniform("rect", glm::vec4((windowW - logoSize - logoMargin)/(GLfloat)windowW, 1.f - (logoMargin+logoSize)/(GLfloat)windowH, logoSize/(GLfloat)windowW, logoSize/(GLfloat)windowH));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        
        //Text rendering
        for(long int i = scrolledLines; i < scrolledLines + visibleLines; i++)
        {
            ConsoleMessage* msg = &lines[linesCount-1-i];
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
            printer->Print(msg->text.c_str(), color, 10.f, scrollOffset + 10.f + i * (STANDARD_FONT_SIZE + 5), STANDARD_FONT_SIZE, true);
        }
    }
    
    
}
    
}
