//
//  IMGUI.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/27/12.
//  Copyright (c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#include "graphics/IMGUI.h"

#include "graphics/Console.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"
#include "utils/stb_image.h"

using namespace sf;

glm::vec4 IMGUI::HSV2RGB(glm::vec4 hsv)
{
    glm::vec4 K = glm::vec4(1.f, 2.f/3.f, 1.f/3.f, 3.f);
    glm::vec3 p = glm::abs(glm::fract(glm::vec3(hsv.x) + glm::vec3(K.x, K.y, K.z)) * 6.f - glm::vec3(K.w));
    return glm::vec4(hsv.z * glm::mix(glm::vec3(K.x), glm::clamp(p - glm::vec3(K.x), 0.f, 1.f), hsv.y), hsv.w);
}

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
    theme[ACTIVE_TEXT_COLOR] = glm::vec4(0.3f, 0.3f, 0.3f, 0.9f);
    theme[INACTIVE_TEXT_COLOR] = glm::vec4(0.7f, 0.7f, 0.7f, 0.5f);
    theme[ACTIVE_CONTROL_COLOR] = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
    theme[INACTIVE_CONTROL_COLOR] = glm::vec4(0.9f, 0.9f, 0.9f, 0.5f);
    theme[HOT_CONTROL_COLOR] = HSV2RGB(glm::vec4(hue, 0.1f, 1.0f, 1.0f));
    theme[PUSHED_CONTROL_COLOR] = HSV2RGB(glm::vec4(hue, 0.2f, 0.8f, 1.0f));
    theme[EMPTY_COLOR] = glm::vec4(0.6f, 0.6f, 0.6f, 0.9f);
    theme[FILLED_COLOR] = HSV2RGB(glm::vec4(hue, 1.f, 0.8f, 0.9f));
    theme[PLOT_COLOR] = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    theme[PLOT_TEXT_COLOR] = glm::vec4(0.9f, 0.9f, 0.9f, 0.9f);
    
    //Set size
    Resize(windowWidth, windowHeight);
    
    //Create printers
    plainPrinter = new OpenGLPrinter(FONT_NAME, FONT_SIZE);
    backgroundMargin = 5.f;
    
    //Load logo texture - can't use material class because it writes to the console
    glActiveTexture(GL_TEXTURE0 + TEX_BASE);
    int width, height, channels;
    std::string path = GetShaderPath() + "logo_gray_64.png";
    
    // Allocate image; fail out on error
    unsigned char* dataBuffer = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if(dataBuffer != NULL)
    {
        // Allocate an OpenGL texture
        glGenTextures(1, &logoTexture);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        // Upload texture to memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);
        // Set certain properties of texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Wrap texture around
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Release internal buffer
        stbi_image_free(dataBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
        logoTexture = 0;
    
    //Load corner texture
    path = GetShaderPath() + "gui.png";
    
    // Allocate image; fail out on error
    dataBuffer = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if(dataBuffer != NULL)
    {
        // Allocate an OpenGL texture
        glGenTextures(1, &guiTexture);
        glBindTexture(GL_TEXTURE_2D, guiTexture);
        // Upload texture to memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);
        // Set certain properties of texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Wrap texture around
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Release internal buffer
        stbi_image_free(dataBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
        guiTexture = 0;
    
    //Generate VAO
    glGenVertexArrays(1, &guiVAO);
    glBindVertexArray(guiVAO);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    
    //Load translucent shaders
    downsampleShader = new GLSLShader("simpleDownsample.frag");
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
    glBindFramebuffer(GL_FRAMEBUFFER, translucentFBO);
    
    glGenTextures(1, &translucentTexture[0]);
    glBindTexture(GL_TEXTURE_2D, translucentTexture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, windowW/4, windowH/4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, translucentTexture[0], 0);
    
    glGenTextures(1, &translucentTexture[1]);
    glBindTexture(GL_TEXTURE_2D, translucentTexture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, windowW/4, windowH/4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, translucentTexture[1], 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        cError("Translucent background FBO initialization failed!");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

ui_id IMGUI::getHot()
{
    return hot;
}

ui_id IMGUI::getActive()
{
    return active;
}

void IMGUI::setHot(ui_id newHot)
{
    hot = newHot;
}

void IMGUI::setActive(ui_id newActive)
{
    active = newActive;
}

bool IMGUI::isHot(ui_id ID)
{
    return (hot.owner == ID.owner && hot.item == ID.item && hot.index == ID.index);
}

bool IMGUI::isActive(ui_id ID)
{
    return (active.owner == ID.owner && active.item == ID.item && active.index == ID.index);
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
    glBindFramebuffer(GL_FRAMEBUFFER, translucentFBO);
	glViewport(0, 0, windowW/4, windowH/4);
    
	glActiveTexture(GL_TEXTURE0 + TEX_BASE);
    glBindTexture(GL_TEXTURE_2D, ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getScreenTexture());
    downsampleShader->Use();
    downsampleShader->SetUniform("source", 0);
    downsampleShader->SetUniform("srcViewport", glm::vec2((GLfloat)windowW, (GLfloat)windowH));
	OpenGLContent::getInstance()->DrawSAQ();
	glUseProgram(0);
    
    for(int i=0; i<3; i++)
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glBindTexture(GL_TEXTURE_2D, translucentTexture[0]);
        
        gaussianShader->Use();
        gaussianShader->SetUniform("source", 0);
        gaussianShader->SetUniform("texelOffset", glm::vec2(4.f/(GLfloat)windowW, 0.f));
        OpenGLContent::getInstance()->DrawSAQ();
        glUseProgram(0);
        
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBindTexture(GL_TEXTURE_2D, translucentTexture[1]);
        
        gaussianShader->Use();
        gaussianShader->SetUniform("source", 0);
        gaussianShader->SetUniform("texelOffset", glm::vec2(0.f, 4.f/(GLfloat)windowH));
        OpenGLContent::getInstance()->DrawSAQ();
        glUseProgram(0);
    }
	
	glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void IMGUI::Begin()
{
	clearHot();
	
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glScissor(0, 0, windowW, windowH);
    glViewport(0, 0, windowW, windowH);
    OpenGLContent::getInstance()->SetViewportSize(windowW, windowH);
	glBindVertexArray(guiVAO);
}

void IMGUI::End()
{
	glBindVertexArray(0);
	
    //draw logo on top
    GLfloat logoSize = 64.f;
    GLfloat logoMargin = 10.f;
	OpenGLContent::getInstance()->DrawTexturedQuad(windowW - logoSize - logoMargin, logoMargin, logoSize, logoSize, logoTexture, glm::vec4(1.f,1.f,1.f,0.2f));
   
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void IMGUI::DrawPlainText(GLfloat x, GLfloat y, glm::vec4 color, const char *text)
{
    plainPrinter->Print(text, color, x, windowH - y - FONT_BASELINE, FONT_SIZE);
}

GLfloat IMGUI::PlainTextLength(const char* text)
{
    return plainPrinter->TextLength(text);
}

glm::vec2 IMGUI::PlainTextDimensions(const char *text)
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
	
	glUseProgram(0);
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
	glActiveTexture(GL_TEXTURE0 + TEX_BASE);
	glBindTexture(GL_TEXTURE_2D, guiTexture);
	
    glActiveTexture(GL_TEXTURE0 + TEX_GUI1);
    glBindTexture(GL_TEXTURE_2D, getTranslucentTexture());
	
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
	
	glUseProgram(0);
	glDeleteBuffers(1, &vbo);
}

void IMGUI::DoPanel(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
}

void IMGUI::DoLabel(GLfloat x, GLfloat y, const char* text, GLfloat* color)
{
    glm::vec4 c;
    
    if(color == NULL)
        c = theme[ACTIVE_TEXT_COLOR];
    else
        c = glm::make_vec4(color);
       
    DrawPlainText(x, y, c, text);
}

bool IMGUI::DoButton(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, const char* title)
{
    bool result = false;
    
    if(MouseInRect(x, y, w, h))
        setHot(ID);
    
    if(isActive(ID))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(ID))
                result = true;
            clearActive();
        }
    }
    else if(isHot(ID))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(ID);
    }
    
    //drawing
    if(isActive(ID))
        DrawRoundedRect(x, y, w, h, theme[PUSHED_CONTROL_COLOR]);
    else if(isHot(ID))
        DrawRoundedRect(x, y, w, h, theme[HOT_CONTROL_COLOR]);
    else
        DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
		
    glm::vec2 textDim = PlainTextDimensions(title);
    DrawPlainText(x + floorf((w - textDim.x)/2.f), y + floorf((h - textDim.y)/2.f), theme[ACTIVE_TEXT_COLOR], title);
    
    return result;
}

btScalar IMGUI::DoSlider(ui_id ID, GLfloat x, GLfloat y, GLfloat w, btScalar min, btScalar max, btScalar value, const char* title)
{
	//Check and correct dimensions
    w = w < 8*backgroundMargin ? 8.f*backgroundMargin : w;
	
	GLfloat railW = w - 4.f*backgroundMargin;
	GLfloat railH = 5.f;
	GLfloat sliderW = 5.f;
	GLfloat sliderH = 20.f;
    GLfloat h = sliderH + 2.f * backgroundMargin + 5.f + FONT_SIZE;
	
    //Check mouse position
    btScalar result = value;
    GLfloat sliderPosition = (value-min)/(max-min);
    
    if(MouseInRect(x + backgroundMargin*2.f + sliderPosition * railW - sliderW/2.f, y + backgroundMargin + FONT_SIZE + 5.f, sliderW, sliderH))
        setHot(ID);
    
    if(isActive(ID))
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
    else if(isHot(ID))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(ID);
    }
    
    //Drawing
    //Background and text
    DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
    DrawPlainText(x + backgroundMargin, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], title);
    
    char buffer[16];
    sprintf(buffer, "%1.2lf", result);
    glm::vec2 textDim = PlainTextDimensions(buffer);
    DrawPlainText(x + railW + 3.f*backgroundMargin - textDim.x, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], buffer);
    
    //Bar
	DrawRect(x+backgroundMargin*2.f, y+backgroundMargin+FONT_SIZE+5.f+sliderH/2.f-railH/2.f, sliderPosition * railW, railH, theme[FILLED_COLOR]);
    DrawRect(x+backgroundMargin*2.f + sliderPosition * railW, y + backgroundMargin+FONT_SIZE+5.f+sliderH/2.f-railH/2.f, railW - sliderPosition*railW, railH, theme[EMPTY_COLOR]);
    
	//Slider
	if(isActive(ID))
        DrawRect(x+backgroundMargin*2.f + sliderPosition * railW - sliderW/2.f, y+backgroundMargin+FONT_SIZE+5.f, sliderW, sliderH, theme[PUSHED_CONTROL_COLOR]);
    else if(isHot(ID))
        DrawRect(x+backgroundMargin*2.f + sliderPosition * railW - sliderW/2.f, y+backgroundMargin+FONT_SIZE+5.f, sliderW, sliderH, theme[HOT_CONTROL_COLOR]);
    else
        DrawRect(x+backgroundMargin*2.f + sliderPosition * railW - sliderW/2.f, y+backgroundMargin+FONT_SIZE+5.f, sliderW, sliderH, theme[ACTIVE_CONTROL_COLOR]);
	
    return result;
}

void IMGUI::DoProgressBar(GLfloat x, GLfloat y, GLfloat w, btScalar progress, const char* title)
{
	//Check and correct dimensions
    w = w < 8*backgroundMargin ? 8.f*backgroundMargin : w;
	GLfloat barW = w - 4.f*backgroundMargin;
	GLfloat barH = 5.f;
	GLfloat h =  barH + 2.f * backgroundMargin + 5.f + FONT_SIZE;
	
    //Check and correct progress value
    progress = progress < btScalar(0.) ? btScalar(0.) : (progress > btScalar(1.) ? btScalar(1.) : progress);
    
    //Draw background and title
    DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
    DrawPlainText(x + backgroundMargin, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], title);
    
    char buffer[16];
    sprintf(buffer, "%1.1lf%%", progress * 100.0);
    GLfloat len = PlainTextLength(buffer);
    DrawPlainText(x + 3.f*backgroundMargin + barW - len, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], buffer);
    
	//Draw bar
	DrawRect(x+backgroundMargin*2.f, y+backgroundMargin+FONT_SIZE+5.f-barH/2.f, progress * barW, barH, theme[FILLED_COLOR]);
    DrawRect(x+backgroundMargin*2.f + progress * barW, y + backgroundMargin+FONT_SIZE+5.f-barH/2.f, barW - progress*barW, barH, theme[EMPTY_COLOR]);
}

bool IMGUI::DoCheckBox(ui_id ID, GLfloat x, GLfloat y, GLfloat w, bool value, const char* title)
{
    bool result = value;
	GLfloat size = 14.f;
	w = w < size + 2.f*backgroundMargin ? size + 2.f*backgroundMargin : w;
	GLfloat h = size + 2.f * backgroundMargin;
    
	if(MouseInRect(x + backgroundMargin, y + backgroundMargin, size, size))
        setHot(ID);
    
    if(isActive(ID))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(ID))
                result = !result;
                
            clearActive();
        }
    }
    else if(isHot(ID))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(ID);
    }
    
    //drawing
    glm::vec2 textDim = PlainTextDimensions(title);
    DrawRoundedRect(x, y, w, h, theme[PANEL_COLOR]);
    DrawPlainText(x + size + backgroundMargin + 5.f, y + backgroundMargin + size/2.f - textDim.y/2.f, theme[ACTIVE_TEXT_COLOR], title);
    
    if(isActive(ID))
        DrawRect(x+backgroundMargin, y+backgroundMargin, size, size, theme[PUSHED_CONTROL_COLOR]);
    else if(isHot(ID))
        DrawRect(x+backgroundMargin, y+backgroundMargin, size, size, theme[HOT_CONTROL_COLOR]);
    else
        DrawRect(x+backgroundMargin, y+backgroundMargin, size, size, theme[ACTIVE_CONTROL_COLOR]);
    
    if(result)
		DrawRect(x+backgroundMargin + 0.2f*size, y+backgroundMargin+0.2f*size, 0.6*size, 0.6*size, theme[FILLED_COLOR]);
    
    return result;
}

bool IMGUI::DoTimePlot(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, ScalarSensor* sens, std::vector<unsigned short>& dims, const char* title, btScalar fixedRange[2])
{
    bool result = false;
	GLfloat pltW = w/windowW * 2.f;
	GLfloat pltH = h/windowH * 2.f;
	GLfloat pltX = x/windowW * 2.f - 1.f;
	GLfloat pltY = (windowH-y)/windowH * 2.f - 1.f;
	GLfloat pltMargin = 10.f/windowH*2.f; 
	
    if(MouseInRect(x, y, w, h))
        setHot(ID);
    
    if(isActive(ID))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(ID))
                result = true;
            clearActive();
        }
    }
    else if(isHot(ID))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(ID);
    }
    
    //Drawing
    DrawRoundedRect(x, y, w, h, theme[PLOT_COLOR]);
    
    //data
    const std::deque<Sample*>& data = sens->getHistory();
    
    if(data.size() > 1)
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
            minValue = 1000.f;
            maxValue = -1000.f;
        
            for(unsigned int i = 0; i < data.size(); i++)
            {
                for(unsigned int n = 0; n < dims.size(); n++)
                {
                    GLfloat value = (GLfloat)data[i]->getValue(dims[n]);
                    if(value > maxValue)
                        maxValue = value;
                    else if(value < minValue)
                        minValue = value;
                }
            }
        
            if(maxValue == minValue) //secure division by zero
            {
                maxValue += 1.f;
                minValue -= 1.f;
            }
        }
        
        GLfloat dy = (pltH-2.f*pltMargin)/(maxValue-minValue);
        
        //autostretch
        GLfloat dt = pltW/(GLfloat)(data.size()-1);
    
        //drawing
        for(unsigned int n = 0; n < dims.size(); n++)
        {
            //set color
            glm::vec4 color;
            
            if(dims.size() > 1)
                color = HSV2RGB(glm::vec4(n/(GLfloat)(dims.size()-1)*0.5, 1.f, 1.f, 1.f));
            else
                color = theme[FILLED_COLOR];
            
            //draw graph
			std::vector<glm::vec2> points;
			for(unsigned long i = 0;  i < data.size(); ++i)
            {
				GLfloat value = (GLfloat)data[i]->getValue(dims[n]);
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
			
			glUseProgram(0);
			glDeleteBuffers(1, &vbo);
			
			//legend
            DrawRect(x - 10.f, y+n*10.f+5.f, 10.f, 10.f, color);
        }
        
        //Grid
		if(minValue < 0 && maxValue > 0)
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
			
			glUseProgram(0);
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
        
    //title
    glm::vec2 titleDim = PlainTextDimensions(title);
    DrawPlainText(x + floorf((w - titleDim.x) / 2.f), y + backgroundMargin, theme[PLOT_TEXT_COLOR], title);
    
    return result;
}

bool IMGUI::DoXYPlot(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, ScalarSensor* sensX, unsigned short dimX, ScalarSensor* sensY, unsigned short dimY, const char* title)
{
    bool result = false;
    GLfloat pltW = w/windowW * 2.f;
	GLfloat pltH = h/windowH * 2.f;
	GLfloat pltX = x/windowW * 2.f - 1.f;
	GLfloat pltY = (windowH-y)/windowH * 2.f - 1.f;
	
    if(MouseInRect(x, y, w, h))
        setHot(ID);
    
    if(isActive(ID))
    {
        if(!MouseIsDown(true)) //mouse went up
        {
            if(isHot(ID))
                result = true;
            clearActive();
        }
    }
    else if(isHot(ID))
    {
        if(MouseIsDown(true)) //mouse went down
            setActive(ID);
    }
    
    //drawing
    //background
    DrawRoundedRect(x, y, w, h, theme[PLOT_COLOR]);
    
    //data
    const std::deque<Sample*>& dataX = sensX->getHistory();
    const std::deque<Sample*>& dataY = sensY->getHistory();
    
    if((dataX.size() > 1) && (dataY.size() > 1))
    {
        //common sample count
        unsigned long dataCount = dataX.size();
        if(dataY.size() < dataCount)
            dataCount = dataY.size();
        
        //autoscale X axis
        GLfloat minValueX = 1000;
        GLfloat maxValueX = -1000;
        
        for(unsigned long i = 0; i < dataCount; ++i)
        {
            GLfloat value = dataX[i]->getValue(dimX);
            if(value > maxValueX)
                maxValueX = value;
            else if(value < minValueX)
                minValueX = value;
        }
        
        if(maxValueX == minValueX) //secure division by zero
        {
            maxValueX += 1.f;
            minValueX -= 1.f;
        }
        
        GLfloat dx = pltW/(maxValueX - minValueX);
        
        //autoscale Y axis
        GLfloat minValueY = 1000;
        GLfloat maxValueY = -1000;
        
        for(unsigned long i = 0; i < dataCount; ++i)
        {
            GLfloat value = dataY[i]->getValue(dimY);
            if(value > maxValueY)
                maxValueY = value;
            else if(value < minValueY)
                minValueY = value;
        }
        
        if(maxValueY == minValueY) //secure division by zero
        {
            maxValueY += 1.f;
            minValueY -= 1.f;
        }
        
        GLfloat dy = pltH/(maxValueY - minValueY);
        
        //draw graph
		std::vector<glm::vec2> points;
		
        for(unsigned long i = 0;  i < dataCount; ++i)
        {
			GLfloat valueX = dataX[i]->getValue(dimX);
            GLfloat valueY = dataY[i]->getValue(dimY);
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
		
		glUseProgram(0);
		glDeleteBuffers(1, &vbo);
		
		//draw axes
		std::vector<glm::vec2> axes;
		
		if(minValueX * maxValueX < 0)
		{
			axes.push_back(glm::vec2(pltX - minValueX * dx, pltY));
			axes.push_back(glm::vec2(pltX - minValueX * dx, pltY - pltH));
		}
		
		if(minValueY * maxValueY < 0)
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
			
			glUseProgram(0);
			glDeleteBuffers(1, &vbo);
		}
    }
    
    //title
    glm::vec2 titleDim = PlainTextDimensions(title);
    DrawPlainText(x + floorf((w - titleDim.x) / 2.f), y + backgroundMargin, theme[PLOT_TEXT_COLOR], title);
    
    return result;
}
