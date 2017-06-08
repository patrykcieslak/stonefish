//
//  IMGUI.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/27/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "IMGUI.h"
#include <stdio.h>
#include <math.h>
#include "SystemUtil.h"
#include "stb_image.h"
#include "Console.h"
#include "OpenGLSolids.h"

IMGUI* IMGUI::instance = NULL;

IMGUI* IMGUI::getInstance()
{
    if(instance == NULL)
        instance = new IMGUI();
    
    return instance;
}

glm::vec4 IMGUI::HSV2RGB(glm::vec4 hsv)
{
    glm::vec4 K = glm::vec4(1.f, 2.f/3.f, 1.f/3.f, 3.f);
    glm::vec3 p = glm::abs(glm::fract(glm::vec3(hsv.x) + glm::vec3(K.x, K.y, K.z)) * 6.f - glm::vec3(K.w));
    return glm::vec4(hsv.z * glm::mix(glm::vec3(K.x), glm::clamp(p - glm::vec3(K.x), 0.f, 1.f), hsv.y), hsv.w);
}

IMGUI::IMGUI()
{
    windowW = windowH = -1; //Not initialized
    shaders = false;
    mouseX = 0;
    mouseY = 0;
    mouseLeftDown = false;
    mouseRightDown = false;
    clearActive();
    translucentFBO = 0;
    translucentTexture[0] = 0;
    translucentTexture[1] = 0;
    downsampleShader = NULL;
    gaussianShader = NULL;
}

IMGUI::~IMGUI()
{
    delete plainPrinter;
    if(logoTexture > 0) glDeleteTextures(1, &logoTexture);
    delete downsampleShader;
    delete gaussianShader;
    glDeleteTextures(2, translucentTexture);
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

void IMGUI::Init(GLint windowWidth, GLint windowHeight, GLfloat hue)
{
    if(windowWidth < 1 || windowHeight < 1)
        return;
    
    bool firstInit = windowW < 0;
    windowW = windowWidth;
    windowH = windowHeight;
    
    //Resize printing area
    OpenGLPrinter::SetWindowSize(windowW, windowH);
    
    //Set interface colors
    theme[PANEL_COLOR] = glm::vec4(0.98f, 0.98f, 0.98f, 1.f);
    theme[ACTIVE_TEXT_COLOR] = glm::vec4(0.7f, 0.7f, 0.7f, 0.9f);
    theme[INACTIVE_TEXT_COLOR] = glm::vec4(0.9f, 0.9f, 0.9f, 0.5f);
    theme[ACTIVE_CONTROL_COLOR] = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
    theme[INACTIVE_CONTROL_COLOR] = glm::vec4(0.9f, 0.9f, 0.9f, 0.5f);
    theme[HOT_CONTROL_COLOR] = HSV2RGB(glm::vec4(hue, 0.1f, 1.0f, 1.0f));
    theme[PUSHED_CONTROL_COLOR] = HSV2RGB(glm::vec4(hue, 0.2f, 0.8f, 1.0f));
    theme[EMPTY_COLOR] = glm::vec4(0.6f, 0.6f, 0.6f, 0.9f);
    theme[FILLED_COLOR] = HSV2RGB(glm::vec4(hue, 1.f, 0.8f, 0.9f));
    theme[PLOT_COLOR] = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    theme[PLOT_TEXT_COLOR] = glm::vec4(0.9f, 0.9f, 0.9f, 0.9f);
    theme[GAUGE_SAFE_COLOR] = glm::vec4(0.6f, 0.6f, 0.6f, 0.9f);
    theme[GAUGE_DANGER_COLOR] = HSV2RGB(glm::vec4(1.f, 1.f, 0.9f, 0.9f));
    
    //Destroy translucent background textures and framebuffers
    if(translucentTexture[0] != 0)
        glDeleteTextures(2, translucentTexture);
    if(translucentFBO != 0)
        glDeleteFramebuffers(1, &translucentFBO);
    
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
    
    if(firstInit)
    {
        //Create printers
        plainPrinter = new OpenGLPrinter(FONT_NAME, FONT_SIZE, SCREEN_DPI);
        backgroundMargin = 5.f;
        
        //Load logo texture - can't use material class because it writes to the console
        char path[1024];
        int width, height, channels;
        GetDataPath(path, 1024-32);
        strcat(path, "logo_gray.png");
        
        // Allocate image; fail out on error
        unsigned char* dataBuffer = stbi_load(path, &width, &height, &channels, 4);
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
        }
        else
            logoTexture = 0;
        
        //Load translucent shaders
        downsampleShader = new GLSLShader("simpleDownsample.frag");
        downsampleShader->AddUniform("source", INT);
        downsampleShader->AddUniform("srcViewport", VEC2);
        gaussianShader = new GLSLShader("gaussianBlur.frag", "gaussianBlur.vert");
        gaussianShader->AddUniform("source", INT);
        gaussianShader->AddUniform("texelOffset", VEC2);
    }
}

void IMGUI::GenerateBackground()
{
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    
    glBindFramebuffer(GL_FRAMEBUFFER, translucentFBO);
    glViewport(0, 0, windowW/4, windowH/4);
    OpenGLSolids::SetupOrtho();
    
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindTexture(GL_TEXTURE_2D, OpenGLPipeline::getInstance()->getDisplayTexture());
    
    downsampleShader->Enable();
    downsampleShader->SetUniform("source", 0);
    downsampleShader->SetUniform("srcViewport", glm::vec2((GLfloat)windowW, (GLfloat)windowH));
    OpenGLSolids::DrawScreenAlignedQuad();
    downsampleShader->Disable();
    
    for(int i=0; i<3; i++)
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glBindTexture(GL_TEXTURE_2D, translucentTexture[0]);
        
        gaussianShader->Enable();
        gaussianShader->SetUniform("source", 0);
        gaussianShader->SetUniform("texelOffset", glm::vec2(4.f/(GLfloat)windowW, 0.f));
        OpenGLSolids::DrawScreenAlignedQuad();
        gaussianShader->Disable();
        
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBindTexture(GL_TEXTURE_2D, translucentTexture[1]);
        
        gaussianShader->Enable();
        gaussianShader->SetUniform("source", 0);
        gaussianShader->SetUniform("texelOffset", glm::vec2(0.f, 4.f/(GLfloat)windowH));
        OpenGLSolids::DrawScreenAlignedQuad();
        gaussianShader->Disable();
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void IMGUI::Begin()
{
    //prepare orthographic projection
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); //for OGLFT
    
    glScissor(0, 0, windowW, windowH);
    glViewport(0, 0, windowW, windowH);
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glm::mat4 proj = glm::ortho(0.f, (GLfloat)windowW, 0.f, (GLfloat)windowH, -1.f, 1.f);
	glLoadMatrixf(glm::value_ptr(proj));
    glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
    
    clearHot();
}

void IMGUI::End()
{
    //draw logo on top
    GLfloat logoSize = 64.f;
    GLfloat logoMargin = 10.f;
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, logoTexture);
    glColor4f(1.f, 1.f, 1.f, 0.1f);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0);
    glVertex2f(windowW - logoSize - logoMargin, windowH - logoMargin);
    glTexCoord2f(0, 1.f);
    glVertex2f(windowW - logoSize - logoMargin, windowH - logoMargin - logoSize);
    glTexCoord2f(1.f, 0);
    glVertex2f(windowW - logoMargin, windowH - logoMargin);
    glTexCoord2f(1.f, 1.f);
    glVertex2f(windowW - logoMargin, windowH - logoMargin - logoSize);
    glEnd();
    
    //revert to previous projection
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
    glMatrixMode(GL_PROJECTION);
	glPopMatrix();
    glPopAttrib();
}

void IMGUI::DrawPlainText(GLfloat x, GLfloat y, glm::vec4 color, const char *text)
{
    plainPrinter->Print(glm::value_ptr(color), x, windowH - y - FONT_BASELINE, FONT_SIZE, text);
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

void IMGUI::DrawRoundedRect(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat radius)
{
    //Check radius and correct if needed
    radius = radius < 0.f ? 0.f : radius;
    
    if((2.f * radius > w) || (2.f * radius > h))
        radius = w < h ? w/2.f : h/2.f;
    
    w -= 2.f * radius;
    h -= 2.f * radius;
    x += radius;
    y += radius;
    
    //Get translucent texture
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, getTranslucentTexture());
    glm::vec2 coord;
    
    //Draw corners
    if(radius > 0.f)
    {
        glm::vec2 center[4] = { glm::vec2(x, windowH - y),
                                glm::vec2(x + w, windowH - y),
                                glm::vec2(x + w, windowH - y - h),
                                glm::vec2(x, windowH - y -h)};
        GLfloat startAngle;
        int divCount = 4;
        
        for(int i = 0; i < 4; i++) //Draw 4 corners
        {
            glBegin(GL_TRIANGLE_FAN);
            coord = center[i];
            glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
            glVertex2fv(glm::value_ptr(coord));
            
            startAngle = M_PI_2 - i * M_PI_2;
            
            for(int h = 0; h <= divCount; h++)
            {
                coord = center[i] + radius * glm::vec2(cosf(startAngle + h/(GLfloat)divCount * M_PI_2), sinf(startAngle + h/(GLfloat)divCount * M_PI_2));
                glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
                glVertex2fv(glm::value_ptr(coord));
            }
            glEnd();
        }
    }
    
    //Draw central part (cross)
    glBegin(GL_TRIANGLE_STRIP);
    coord = glm::vec2(x, windowH - y + radius);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x + w, windowH - y + radius);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x, windowH - y);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x + w, windowH - y);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x - radius, windowH - y);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x + w + radius, windowH - y);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x - radius, windowH - y - h);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x + w + radius, windowH - y - h);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x, windowH - y - h);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x + w, windowH - y - h);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x, windowH - y - h - radius);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    
    coord = glm::vec2(x + w, windowH - y - h - radius);
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    glEnd();
}

void IMGUI::DoPanel(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat radius)
{
    if(radius > 0.f)
    {
        glColor4fv(glm::value_ptr(theme[PANEL_COLOR]));
        DrawRoundedRect(x, y, w, h, radius);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, getTranslucentTexture());
    
        glColor4fv(glm::value_ptr(theme[PANEL_COLOR]));
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(x/(GLfloat)windowW, 1.f - y/(GLfloat)windowH);
        glVertex2f(x, windowH - y);
        glTexCoord2f((x + w)/(GLfloat)windowW, 1.f - y/(GLfloat)windowH);
        glVertex2f(x + w, windowH - y);
        glTexCoord2f(x/(GLfloat)windowW, 1.f - (y + h)/(GLfloat)windowH);
        glVertex2f(x, windowH - y - h);
        glTexCoord2f((x + w)/(GLfloat)windowW, 1.f - (y + h)/(GLfloat)windowH);
        glVertex2f(x + w, windowH - y - h);
        glEnd();
    }
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
        {
            setActive(ID);
        }
    }
    
    //drawing
    if(isActive(ID))
        glColor4fv(glm::value_ptr(theme[PUSHED_CONTROL_COLOR]));
    else if(isHot(ID))
        glColor4fv(glm::value_ptr(theme[HOT_CONTROL_COLOR]));
    else
        glColor4fv(glm::value_ptr(theme[PANEL_COLOR]));
    
    DrawRoundedRect(x, y, w, h, 5.f);
    
    glm::vec2 textDim = PlainTextDimensions(title);
    DrawPlainText(x + floorf((w - textDim.x)/2.f), y + floorf((h - textDim.y)/2.f), theme[ACTIVE_TEXT_COLOR], title);
    
    return result;
}

btScalar IMGUI::DoSlider(ui_id ID, GLfloat x, GLfloat y, GLfloat railW, GLfloat railH, GLfloat sliderW, GLfloat sliderH, btScalar min, btScalar max, btScalar value, const char* title)
{
    //Check and correct dimensions
    if(railH > sliderH)
        railH = sliderH;
    if(railW < sliderW)
        railW = sliderW;
    
    //Check mouse position
    btScalar result = value;
    GLfloat sliderPosition = (value-min)/(max-min);
    
    if(MouseInRect(x + backgroundMargin + sliderPosition * railW - sliderW/2.f, y + backgroundMargin + FONT_SIZE + 5.f, sliderW, sliderH))
        setHot(ID);
    
    if(isActive(ID))
    {
        GLfloat mouseX = getMouseX();
        if(mouseX <= x + backgroundMargin)
            sliderPosition = 0;
        else if(mouseX >= x + backgroundMargin + railW)
            sliderPosition = 1.f;
        else
            sliderPosition = (mouseX - x - backgroundMargin)/railW;
        
        result = min + sliderPosition * (max - min);
        
        if(!MouseIsDown(true)) //mouse went up
        {
            clearActive();
        }
    }
    else if(isHot(ID))
    {
        if(MouseIsDown(true)) //mouse went down
        {
            setActive(ID);
        }
    }
    
    //Drawing
    //background and text
    glColor4fv(glm::value_ptr(theme[PANEL_COLOR]));
    DrawRoundedRect(x, y, railW + 2.f * backgroundMargin, sliderH + 2.f * backgroundMargin + 5.f + FONT_SIZE, 5.f);
    DrawPlainText(x + backgroundMargin, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], title);
    
    char buffer[16];
    sprintf(buffer, "%1.2lf", result);
    glm::vec2 textDim = PlainTextDimensions(buffer);
    DrawPlainText(x + railW + backgroundMargin - textDim.x, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], buffer);
    
    //Bar
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLE_STRIP);
    glColor4fv(glm::value_ptr(theme[FILLED_COLOR]));
    glVertex2f(x + backgroundMargin, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH/2.f + railH/2.f);
    glVertex2f(x + backgroundMargin, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH/2.f - railH/2.f);
    glVertex2f(x + backgroundMargin + sliderPosition * railW, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH/2.f + railH/2.f);
    glVertex2f(x + backgroundMargin + sliderPosition * railW, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH/2.f - railH/2.f);
    
    glColor4fv(glm::value_ptr(theme[EMPTY_COLOR]));
    glVertex2f(x + backgroundMargin + sliderPosition * railW, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH/2.f + railH/2.f);
    glVertex2f(x + backgroundMargin + sliderPosition * railW, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH/2.f - railH/2.f);
    glVertex2f(x + backgroundMargin + railW, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH/2.f + railH/2.f);
    glVertex2f(x + backgroundMargin + railW, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH/2.f - railH/2.f);
    glEnd();
    
    //Slider
    if(isActive(ID))
        glColor4fv(glm::value_ptr(theme[PUSHED_CONTROL_COLOR]));
    else if(isHot(ID))
        glColor4fv(glm::value_ptr(theme[HOT_CONTROL_COLOR]));
    else
        glColor4fv(glm::value_ptr(theme[ACTIVE_CONTROL_COLOR]));
    
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(x + backgroundMargin + sliderPosition * railW - sliderW/2.f, windowH - y - backgroundMargin - FONT_SIZE - 5.f);
    glVertex2f(x + backgroundMargin + sliderPosition * railW + sliderW/2.f, windowH - y - backgroundMargin - FONT_SIZE - 5.f);
    glVertex2f(x + backgroundMargin + sliderPosition * railW - sliderW/2.f, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH);
    glVertex2f(x + backgroundMargin + sliderPosition * railW + sliderW/2.f, windowH - y - backgroundMargin - FONT_SIZE - 5.f - sliderH);
    glEnd();

    return result;
}

void IMGUI::DoProgressBar(GLfloat x, GLfloat y, GLfloat barW, GLfloat barH, btScalar progress, const char* title)
{
    //Check and correct progress value
    progress = progress < btScalar(0.) ? btScalar(0.) : (progress > btScalar(1.) ? btScalar(1.) : progress);
    
    //Draw background and title
    glColor4fv(glm::value_ptr(theme[PANEL_COLOR]));
    DrawRoundedRect(x, y, barW + 2.f * backgroundMargin, barH + 2.f * backgroundMargin + 5.f + FONT_SIZE, 5.f);
    DrawPlainText(x + backgroundMargin, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], title);
    
    char buffer[16];
    sprintf(buffer, "%1.1lf%%", progress * 100.0);
    GLfloat len = PlainTextLength(buffer);
    DrawPlainText(x + backgroundMargin + barW - len, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], buffer);
    
    //Draw bar
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLE_STRIP);
    glColor4fv(glm::value_ptr(theme[FILLED_COLOR]));
    glVertex2f(x + backgroundMargin, windowH - y - backgroundMargin - FONT_SIZE - 5.f);
    glVertex2f(x + backgroundMargin, windowH - y - backgroundMargin - FONT_SIZE - 5.f - barH);
    glVertex2f(x + backgroundMargin + progress * barW, windowH - y - backgroundMargin - FONT_SIZE - 5.f);
    glVertex2f(x + backgroundMargin + progress * barW, windowH - y - backgroundMargin - FONT_SIZE - 5.f - barH);
    
    glColor4fv(glm::value_ptr(theme[EMPTY_COLOR]));
    glVertex2f(x + backgroundMargin + progress * barW, windowH - y - backgroundMargin - FONT_SIZE - 5.f);
    glVertex2f(x + backgroundMargin + progress * barW, windowH - y - backgroundMargin - FONT_SIZE - 5.f - barH);
    glVertex2f(x + backgroundMargin + barW, windowH - y - backgroundMargin - FONT_SIZE - 5.f);
    glVertex2f(x + backgroundMargin + barW, windowH - y - backgroundMargin - FONT_SIZE - 5.f - barH);
    glEnd();
}

bool IMGUI::DoCheckBox(ui_id ID, GLfloat x, GLfloat y, bool value, const char* title)
{
    bool result = value;
    GLfloat size = 14.f;
    
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
        {
            setActive(ID);
        }
    }
    
    //drawing
    glm::vec2 textDim = PlainTextDimensions(title);
    glColor4fv(glm::value_ptr(theme[PANEL_COLOR]));
    DrawRoundedRect(x, y, size + textDim.x + 3.f * backgroundMargin, size + 2.f * backgroundMargin, 5.f);
    DrawPlainText(x + size + backgroundMargin + 5.f, y + backgroundMargin + size/2.f - textDim.y/2.f, theme[ACTIVE_TEXT_COLOR], title);
    
    glDisable(GL_TEXTURE_2D);
    
    if(isActive(ID))
        glColor4fv(glm::value_ptr(theme[PUSHED_CONTROL_COLOR]));
    else if(isHot(ID))
        glColor4fv(glm::value_ptr(theme[HOT_CONTROL_COLOR]));
    else
        glColor4fv(glm::value_ptr(theme[ACTIVE_CONTROL_COLOR]));
    
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(x + backgroundMargin, windowH - y - backgroundMargin);
    glVertex2f(x + backgroundMargin, windowH - y - size - backgroundMargin);
    glVertex2f(x + size + backgroundMargin, windowH - y - backgroundMargin);
    glVertex2f(x + size + backgroundMargin, windowH - y - size - backgroundMargin);
    glEnd();
    
    if(result)
    {
        glColor4fv(glm::value_ptr(theme[FILLED_COLOR]));
        glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(x + 0.2f * size + backgroundMargin, windowH - y - 0.2f * size - backgroundMargin);
        glVertex2f(x + 0.2f * size + backgroundMargin, windowH - y - 0.8f * size - backgroundMargin);
        glVertex2f(x + 0.8f * size + backgroundMargin, windowH - y - 0.2f * size - backgroundMargin);
        glVertex2f(x + 0.8f * size + backgroundMargin, windowH - y - 0.8f * size - backgroundMargin);
        glEnd();
    }
    
    return result;
}

unsigned short IMGUI::DoRadioGroup(ui_id ID, GLfloat x, GLfloat y, unsigned short selection, std::vector<std::string>& items, const char *title)
{
    unsigned short result = selection;
    GLfloat size = 14.f;
    GLfloat topOffset = title == NULL ? 0.f : FONT_SIZE + 5.f;
    
    //Check if mouse interacts with any item
    for(unsigned int i = 0; i < items.size(); i++)
    {
        ID.index = i;
        
        if(MouseInRect(x + backgroundMargin, y + backgroundMargin + i * (size + 10.f) + topOffset, size, size))
            setHot(ID);
        
        if(isActive(ID))
        {
            if(!MouseIsDown(true)) //mouse went up
            {
                if(isHot(ID))
                    result = i;
                
                clearActive();
            }
        }
        else if(isHot(ID))
        {
            if(MouseIsDown(true)) //mouse went down
            {
                setActive(ID);
            }
        }
    }
    
    //Drawing
    //Background and title/items text
    GLfloat maxLen = 0;
    GLfloat len;
    for(unsigned int i = 0; i < items.size(); i++)
        if((len = PlainTextLength(items[i].c_str())) > maxLen)
            maxLen = len;
    
    glColor4fv(glm::value_ptr(theme[PANEL_COLOR]));
    DrawRoundedRect(x, y, 2.f * backgroundMargin + size + 5.f + maxLen, 2.f * backgroundMargin + topOffset + items.size() * (size + 10.f) - 10.f, 5.f);
    
    for(unsigned int i = 0; i < items.size(); i++)
    {
        glm::vec2 textDim = PlainTextDimensions(items[i].c_str());
        DrawPlainText(x + backgroundMargin + size + 5.f, y + backgroundMargin + topOffset + size/2.f + i * (size + 10.f) - textDim.y/2.f, theme[ACTIVE_TEXT_COLOR], items[i].c_str());
    }
    
    if(title != NULL)
    {
        len = PlainTextLength(title);
        DrawPlainText(x + (2.f * backgroundMargin + size + 5.f + maxLen - len)/2.f, y + backgroundMargin, theme[ACTIVE_TEXT_COLOR], title);
    }
    
    //Items
    glDisable(GL_TEXTURE_2D);
    int divCount = 12;
    
    for(unsigned int i = 0; i < items.size(); i++)
    {
        ID.index = i;
        
        if(isActive(ID))
            glColor4fv(glm::value_ptr(theme[PUSHED_CONTROL_COLOR]));
        else if(isHot(ID))
            glColor4fv(glm::value_ptr(theme[HOT_CONTROL_COLOR]));
        else
            glColor4fv(glm::value_ptr(theme[ACTIVE_CONTROL_COLOR]));
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x + backgroundMargin + size/2.0f, windowH - y - backgroundMargin - topOffset - i * (size + 10.f) - size/2.f);
        for(unsigned int h = 0; h <= divCount; h++)
            glVertex2f(x + backgroundMargin + size/2.f + size/2.f * sinf(h/(GLfloat)divCount * M_PI * 2.f), windowH - y - backgroundMargin - topOffset - i * (size + 10.f) - size/2.f - (size/2.f * cosf(h/(GLfloat)divCount * M_PI * 2.f)));
        glEnd();
        
        if(i == result)
        {
            glColor4fv(glm::value_ptr(theme[FILLED_COLOR]));
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x + backgroundMargin + size/2.f, windowH - y - backgroundMargin - topOffset - i * (size + 10.f) - size/2.f);
            for(unsigned int h = 0; h <= divCount; h++)
                glVertex2f(x + backgroundMargin + size/2.f + (size/2.f * 0.7f) * sinf(h/(GLfloat)divCount * M_PI * 2.f), windowH - y - backgroundMargin - topOffset - i * (size + 10.f) - size/2.f - ((size/2.f * 0.7f) * cosf(h/(GLfloat)divCount * M_PI * 2.f)));
            glEnd();
        }
    }
    
    return result;
}

void IMGUI::DoGauge(GLfloat x, GLfloat y, GLfloat diameter, btScalar value, btScalar range[2], btScalar safeRange[2], const char* title)
{
    //Background and text
    //Get translucent texture
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, getTranslucentTexture());
    
    int divCount = 24;
    glm::vec2 coord;
    glm::vec2 center(x + diameter/2.f, windowH - y - diameter/2.f);
   
    glColor4fv(glm::value_ptr(theme[PANEL_COLOR]));
    glBegin(GL_TRIANGLE_FAN);
    coord = center;
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
            
    for(int i = 0; i <= divCount; i++)
    {
        coord = center + diameter/2.f * glm::vec2(cosf(i/(GLfloat)divCount * 2.f * M_PI), sinf(i/(GLfloat)divCount * 2.f * M_PI));
        glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
        glVertex2fv(glm::value_ptr(coord));
    }
    glEnd();
    
    char buffer[16];
    sprintf(buffer, "%1.1lf", value);
    GLfloat len = PlainTextLength(buffer);
    DrawPlainText(x + diameter/2.f - len/2.f, y + diameter/2.f + 20.f, theme[ACTIVE_TEXT_COLOR], buffer);
    len = PlainTextLength(title);
    DrawPlainText(x + diameter/2.f - len/2.f, y + diameter/2.f + 20.f + FONT_SIZE, theme[ACTIVE_TEXT_COLOR], title);
    
    //Scale
    GLfloat angleRange = M_PI + M_PI_2;
    
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= divCount; i++)
    {
        btScalar value = (1.0 - i/(btScalar)divCount) * (range[1] - range[0]) + range[0];
        
        if(value < safeRange[0] || value > safeRange[1])
            glColor4fv(glm::value_ptr(theme[GAUGE_DANGER_COLOR]));
        else
            glColor4fv(glm::value_ptr(theme[GAUGE_SAFE_COLOR]));
        
        coord = center + diameter/2.f * 0.85f * glm::vec2(cosf(i/(GLfloat)divCount * angleRange - M_PI/4.f), sinf(i/(GLfloat)divCount * angleRange - M_PI/4.f));
        glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
        glVertex2fv(glm::value_ptr(coord));
        coord = center + diameter/2.f * 0.75f * glm::vec2(cosf(i/(GLfloat)divCount * angleRange - M_PI/4.f), sinf(i/(GLfloat)divCount * angleRange - M_PI/4.f));
        glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
        glVertex2fv(glm::value_ptr(coord));
    }
    glEnd();
    
    //Arm
    GLfloat angle = (1.f - (value - range[0])/(range[1] - range[0])) * angleRange;
    
    glColor4fv(glm::value_ptr(theme[FILLED_COLOR]));
    glBegin(GL_TRIANGLES);
    coord = center + diameter/2.f * 0.9f * glm::vec2(cosf(angle - M_PI/4.f), sinf(angle - M_PI/4.f));
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    coord = center + diameter/2.f * 0.1f * glm::vec2(cosf(angle - M_PI/4.f - M_PI_2), sinf(angle - M_PI/4.f - M_PI_2));
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    coord = center + diameter/2.f * 0.1f * glm::vec2(cosf(angle - M_PI/4.f + M_PI), sinf(angle - M_PI/4.f + M_PI));
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    coord = center + diameter/2.f * 0.9f * glm::vec2(cosf(angle - M_PI/4.f), sinf(angle - M_PI/4.f));
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    coord = center + diameter/2.f * 0.1f * glm::vec2(cosf(angle - M_PI/4.f + M_PI_2), sinf(angle - M_PI/4.f + M_PI_2));
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    coord = center + diameter/2.f * 0.1f * glm::vec2(cosf(angle - M_PI/4.f + M_PI), sinf(angle - M_PI/4.f + M_PI));
    glTexCoord2f(coord.x/(GLfloat)windowW, coord.y/(GLfloat)windowH);
    glVertex2fv(glm::value_ptr(coord));
    glEnd();
}

bool IMGUI::DoTimePlot(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, SimpleSensor* sens, std::vector<unsigned short>& dims, const char* title, bool plottingEnabled, btScalar fixedRange[2], unsigned int historyLength)
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
        {
            setActive(ID);
        }
    }
    
    //drawing
    glColor4fv(glm::value_ptr(theme[PLOT_COLOR]));
    DrawRoundedRect(x, y, w, h, 5.f);
    glDisable(GL_TEXTURE_2D);
    
    //data
    const std::deque<Sample*>& data = sens->getHistory();
    
    if(plottingEnabled && data.size() > 1)
    {
        //autoscale
        btScalar minValue = 1000;
        btScalar maxValue = -1000;
        
        for(int i = 0; i < data.size(); i++)
        {
            for(int n = 0; n < dims.size(); n++)
            {
                btScalar value = data[i]->getValue(dims[n]);
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
        
        GLfloat dy = (h-20.f)/(maxValue-minValue);
        
        //autostretch
        GLfloat dt = w/(GLfloat)(data.size()-1);
    
        //drawing
        for(int n = 0; n < dims.size(); n++)
        {
            //set color
            glm::vec4 color;
            
            if(dims.size() > 1)
                color = HSV2RGB(glm::vec4(n/(GLfloat)(dims.size()-1)*0.5, 1.f, 1.f, 1.f));
            else
                color = theme[FILLED_COLOR];
            
            //draw graph
            glColor4fv(glm::value_ptr(color));
            glBegin(GL_LINE_STRIP);
            for(int i = 0;  i < data.size(); i++)
            {
                btScalar value = data[i]->getValue(dims[n]);
                glVertex2f(x + dt*i, windowH - y - (h-10.f) + (value-minValue)*dy);
            }
            glEnd();
            
            //legend
            glBegin(GL_TRIANGLE_STRIP);
            glVertex2f(x -10.f, windowH - y  - n * 10.f - 5.f);
            glVertex2f(x, windowH - y - n * 10.f - 5.f);
            glVertex2f(x -10.f, windowH - y - (n + 1) * 10.f - 5.f);
            glVertex2f(x, windowH - y - (n + 1) * 10.f - 5.f);
            glEnd();
        }
        
        glColor4fv(glm::value_ptr(theme[PLOT_TEXT_COLOR]));
        glBegin(GL_LINES);
        if(minValue < 0 && maxValue > 0)
        {
            glVertex2f(x, windowH - y - (h-10.f) - minValue * dy);
            glVertex2f(x + w, windowH - y - (h-10.f) - minValue * dy);
        }
        glEnd();
        
        //Check if mouse cursor above legend item and display signal info
        if(MouseInRect(x - 10.f, y + 5.f, 10.f, dims.size() * 10.f)) //mouse above legend
        {
            long selectedDim = (long)floorf((mouseY - y - 5.f)/10.f);
            if(selectedDim < 0)
                selectedDim = 0;
            if(selectedDim > dims.size()-1)
                selectedDim = dims.size()-1;
            
            char buffer[64];
            sprintf(buffer, "%1.6f", sens->getLastSample().getValue(dims[selectedDim]));

            glEnable(GL_TEXTURE_2D);
            DrawPlainText(x + backgroundMargin, y + backgroundMargin, theme[PLOT_TEXT_COLOR], buffer);
        }
    }
        
    //title
    glEnable(GL_TEXTURE_2D);
    glm::vec2 titleDim = PlainTextDimensions(title);
    DrawPlainText(x + floorf((w - titleDim.x) / 2.f), y + backgroundMargin, theme[PLOT_TEXT_COLOR], title);
    
    return result;
}

bool IMGUI::DoXYPlot(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, SimpleSensor* sensX, unsigned short dimX, SimpleSensor* sensY, unsigned short dimY, const char* title, unsigned int historyLength)
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
        {
            setActive(ID);
        }
    }
    
    //drawing
    //background
    glColor4fv(glm::value_ptr(theme[PLOT_COLOR]));
    DrawRoundedRect(x, y, w, h, 5.f);
    glDisable(GL_TEXTURE_2D);
    
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
        btScalar minValueX = 1000;
        btScalar maxValueX = -1000;
        
        for(unsigned long i = 0; i < dataCount; i++)
        {
            btScalar value = dataX[i]->getValue(dimX);
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
        
        GLfloat dx = w/(maxValueX - minValueX);
        
        //autoscale Y axis
        btScalar minValueY = 1000;
        btScalar maxValueY = -1000;
        
        for(unsigned long i = 0; i < dataCount; i++)
        {
            btScalar value = dataY[i]->getValue(dimY);
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
        
        GLfloat dy = h/(maxValueY - minValueY);
        
        //draw graph
        glColor4fv(glm::value_ptr(theme[FILLED_COLOR]));
        glBegin(GL_LINE_STRIP);
        for(unsigned long i = 0;  i < dataCount; i++)
        {
            btScalar valueX = dataX[i]->getValue(dimX);
            btScalar valueY = dataY[i]->getValue(dimY);
            glVertex2f(x + (valueX - minValueX) * dx, windowH - y - h + (valueY - minValueY) * dy);
        }
        glEnd();
        
        glColor4fv(glm::value_ptr(theme[PLOT_TEXT_COLOR]));
        glBegin(GL_LINES);
        if(minValueX * maxValueX < 0)
        {
            glVertex2f(x - minValueX * dx, windowH - y - h);
            glVertex2f(x - minValueX * dx, windowH - y);
        }
        if(minValueY * maxValueY < 0)
        {
            glVertex2f(x, windowH - y - h - minValueY * dy);
            glVertex2f(x + w, windowH - y - h - minValueY * dy);
        }
        glEnd();
    }
    
    //title
    glEnable(GL_TEXTURE_2D);
    glm::vec2 titleDim = PlainTextDimensions(title);
    DrawPlainText(x + floorf((w - titleDim.x) / 2.f), y + backgroundMargin, theme[PLOT_TEXT_COLOR], title);
    
    return result;
}
