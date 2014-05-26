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

static GLfloat ui_colors[46][4] =
{
    //Panel
    {0.8f,0.8f,0.8f,0.6f},
    {0.2f,0.22f,0.25f,0.7f},
    {0.3f,0.32f,0.35f,0.7f},
    {0.3f,0.32f,0.35f,0.7f},
    {0.2f,0.22f,0.25f,0.7f},
    {0.8f,0.8f,0.8f,1.0f},
    //Button
    {0.8f,0.8f,0.8f,1.0f},
    {0.85f,0.85f,0.85f,1.0f},
    {0.9f,0.9f,0.9f,1.0f},
    {0.2f,0.22f,0.25f,0.7f},
    {0.2f,0.22f,0.25f,1.0f},
    //Slider
    {0.2f, 0.2f, 0.2f, 1.f},
    {1.f, 0.4f, 0.1f, 1.f},
    {1.0f,1.0f,1.0f,1.0f},
    {1.0f,1.0f,1.0f,1.0f},
    {1.0f,1.0f,1.0f,1.0f},
    {1.0f,1.0f,1.0f,0.7f},
    {1.0f,1.0f,1.0f,1.0f},
    {1.0f,1.0f,1.0f,1.0f},
    //Progress bar
    {0.2f, 0.22f, 0.25f, 1.f},
    {1.f, 0.95f, 0.1f, 1.f},
    {0.2f,0.22f,0.25f,0.7f},
    {1.0f,1.0f,1.0f,1.0f},
    {1.0f,1.0f,1.0f,1.0f},
    //Check box
    {0.8f,0.8f,0.8f,1.0f},
    {0.85f,0.85f,0.85f,1.0f},
    {0.9f,0.9f,0.9f,1.0f},
    {0.2f,0.22f,0.25f,0.7f},
    {0.2f,0.22f,0.25f,1.0f},
    {0.2f,0.22f,0.25f,1.0f},
    //Radio button
    {0.8f,0.8f,0.8f,1.0f},
    {0.85f,0.85f,0.85f,1.0f},
    {0.9f,0.9f,0.9f,1.0f},
    {0.2f,0.22f,0.25f,0.7f},
    {0.2f,0.22f,0.25f,1.0f},
    {0.2f,0.22f,0.25f,1.0f},
    //Plots
    {0.2f,0.2f,0.2f,0.8f},
    {1.0f,1.0f,1.0f,1.0f},
    {0.8f,0.8f,0.8f,1.0f},
    {0.5f,0.5f,0.5f,0.8f},
    {1.0f,1.0f,1.0f,1.0f}, //PLOT_DATA_COLOR1
    {1.0f,0.3f,0.3f,1.0f},
    {0.3f,1.0f,0.3f,1.0f},
    {0.3f,0.3f,1.0f,1.0f},
    {1.0f,1.0f,0.3f,1.0f},
    {0.3f,1.0f,1.0f,1.0f}
};

IMGUI::IMGUI()
{
    windowW = windowH = 200;
    shaders = false;
    mouseX = 0;
    mouseY = 0;
    mouseLeftDown = false;
    mouseRightDown = false;
    clearActive();
}

IMGUI::~IMGUI()
{
    delete plainPrinter;
}

void IMGUI::SetRenderSize(int width, int height)
{
    windowW = width;
    windowH = height;
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

void IMGUI::Init()
{
    OpenGLPrinter::SetWindowSize(windowW, windowH);
    plainPrinter = new OpenGLPrinter(FONT_NAME, FONT_SIZE, SCREEN_DPI);
}

void IMGUI::Begin()
{
    //prepare orthographic projection
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable (GL_LIGHTING);
    glActiveTextureARB(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_POLYGON_SMOOTH);
	//glEnable(GL_LINE_SMOOTH);
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(1.f);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); //for OGLFT
    
    glScissor(0, 0, windowW, windowH);
    glViewport(0, 0, windowW, windowH);
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLfloat)windowW, 0.0, (GLfloat)windowH, -100.f, 100.f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
    
    clearHot();
}

void IMGUI::End()
{
    //revert to previous projection
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
    glMatrixMode(GL_PROJECTION);
	glPopMatrix();
    glPopAttrib();
}

void IMGUI::DrawPlainText(GLfloat x, GLfloat y, GLfloat *color, const char *text)
{
    plainPrinter->Print(color, x, y, FONT_SIZE, text);
}

GLfloat IMGUI::PlainTextLength(const char* text)
{
    return plainPrinter->TextLength(text);
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

//static
void IMGUI::DoLabel(ui_id ID, GLfloat x, GLfloat y, GLfloat* color, const char* text)
{
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m);
    
    glEnable(GL_TEXTURE_2D);
    DrawPlainText(x+m[12], windowH-y-m[13], color, text);
}

void IMGUI::DoPanel(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, const char* title)
{
    glLoadIdentity();
    //glScissor(x, getWindowHeight()-y-h, w, getWindowHeight()-y-25.f);
    
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLE_STRIP);
    glColor4fv(ui_colors[PANEL_TOP_BAR_COLOR]);
    glVertex2f(x, windowH - y);
    glColor4fv(ui_colors[PANEL_TOP_BAR_COLOR]);
    glVertex2f(x + w, windowH - y);
    glColor4fv(ui_colors[PANEL_TOP_BAR_COLOR]);
    glVertex2f(x, windowH - y - 3.f);
    glColor4fv(ui_colors[PANEL_TOP_BAR_COLOR]);
    glVertex2f(x+w, windowH - y - 3.f);
    
    glColor4fv(ui_colors[PANEL_LEFT_BAR_COLOR]);
    glVertex2f(x, windowH - y - 3.f);
    glColor4fv(ui_colors[PANEL_RIGHT_BAR_COLOR]);
    glVertex2f(x+w, windowH - y - 3.f);
    glColor4fv(ui_colors[PANEL_LEFT_BAR_COLOR]);
    glVertex2f(x, windowH - y - 25.f);
    glColor4fv(ui_colors[PANEL_RIGHT_BAR_COLOR]);
    glVertex2f(x+w, windowH - y - 25.f);
    
    glColor4fv(ui_colors[PANEL_BACKGROUND_COLOR]);
    glVertex2f(x, windowH - y -25.f);
    glVertex2f(x+w, windowH - y -25.f);
    glVertex2f(x, windowH - y -h);
    glVertex2f(x+w, windowH - y -h);
    glEnd();
    
    glColor4fv(ui_colors[PANEL_BORDER_COLOR]);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, windowH - y);
    glVertex2f(x+w+1.f, windowH - y);
    glVertex2f(x+w+1.f, windowH - y - h -1.f);
    glVertex2f(x, windowH - y - h - 1.f);
    glVertex2f(x, windowH - y + 1.f);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    DrawPlainText(x+5, windowH - y - 8.f, ui_colors[PANEL_TITLE_COLOR], title);
    
    glTranslatef(x, windowH - y - 25.f, 0);
}

bool IMGUI::DoButton(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, const char* title)
{
    bool result = false;
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m); //12, 13, 14 -> x,y,z
    
    if(MouseInRect(x+m[12], y+m[13], w, h))
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
    glDisable(GL_TEXTURE_2D);
    
    if(isActive(ID))
        glColor4fv(ui_colors[BUTTON_ACTIVE_COLOR]);
    else if(isHot(ID))
        glColor4fv(ui_colors[BUTTON_HOT_COLOR]);
    else
        glColor4fv(ui_colors[BUTTON_NORMAL_COLOR]);
    
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(x, windowH-y);
    glVertex2f(x, windowH-y-h);
    glVertex2f(x+w, windowH-y);
    glVertex2f(x+w, windowH-y-h);
    glEnd();
    
    glColor4fv(ui_colors[BUTTON_BORDER_COLOR]);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, windowH - y);
    glVertex2f(x+w+1.f, windowH-y+1.f);
    glVertex2f(x+w+1.f, windowH-y-h-1.f);
    glVertex2f(x, windowH-y-h-1.f);
    glVertex2f(x, windowH - y + 1.f);
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
    GLfloat len = PlainTextLength(title);
	
    DrawPlainText(x+floorf((w-len)/2.f), windowH-y-h/2.f+6.f, ui_colors[BUTTON_TITLE_COLOR], title);
    
    return result;
}

double IMGUI::DoSlider(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat sliderW, GLfloat sliderH, double min, double max, double value, const char* title)
{
    double result = value;
    GLfloat sliderPosition = (value-min)/(max-min);
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m); //12, 13, 14 -> x,y,z
    
    if(MouseInRect(x+m[12]+sliderPosition*w-sliderW/2.f, y+m[13]-sliderH/2.f, sliderW, sliderH))
        setHot(ID);
    
    if(isActive(ID))
    {
        GLfloat mouseX = getMouseX();
        if(mouseX <= x+m[12])
            sliderPosition = 0;
        else if(mouseX >=x+m[12]+w)
            sliderPosition = 1.f;
        else
            sliderPosition = (mouseX-x-m[12])/w;
        
        result = min+sliderPosition*(max-min);
        
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
    
    //drawing
    glDisable(GL_TEXTURE_2D);
    
    glBegin(GL_TRIANGLE_STRIP);
    glColor4fv(ui_colors[SLIDER_BAR_FILLED_COLOR]);
    glVertex2f(x, windowH-y+2.f);
    glVertex2f(x, windowH-y-2.f);
    glVertex2f(x+sliderPosition*w, windowH-y+2.f);
    glVertex2f(x+sliderPosition*w, windowH-y-2.f);
    
    glColor4fv(ui_colors[SLIDER_BAR_EMPTY_COLOR]);
    glVertex2f(x+sliderPosition*w, windowH-y+2.f);
    glVertex2f(x+sliderPosition*w, windowH-y-2.f);
    glVertex2f(x+w, windowH-y+2.f);
    glVertex2f(x+w, windowH-y-2.f);
    glEnd();
    
    if(isActive(ID))
        glColor4fv(ui_colors[SLIDER_ACTIVE_COLOR]);
    else if(isHot(ID))
        glColor4fv(ui_colors[SLIDER_HOT_COLOR]);
    else
        glColor4fv(ui_colors[SLIDER_NORMAL_COLOR]);
    
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(x+sliderPosition*w-sliderW/2.f, windowH-y+sliderH/2.f);
    glVertex2f(x+sliderPosition*w+sliderW/2.f, windowH-y+sliderH/2.f);
    glVertex2f(x+sliderPosition*w-sliderW/2.f, windowH-y-sliderH/2.f);
    glVertex2f(x+sliderPosition*w+sliderW/2.f, windowH-y-sliderH/2.f);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    DrawPlainText(x-2.0, windowH - y + 15.f + sliderH/2.f, ui_colors[SLIDER_TITLE_COLOR], title);
    
    char buffer[16];
    sprintf(buffer, "%1.2lf", result);
    GLfloat len = PlainTextLength(buffer);
    DrawPlainText(x-4.0 + w - len, windowH - y + 15.f + sliderH/2.f, ui_colors[SLIDER_VALUE_COLOR], buffer);
    
    return result;
}

void IMGUI::DoProgressBar(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, double progress, const char* title)
{
    glDisable(GL_TEXTURE_2D);
    
    glBegin(GL_TRIANGLE_STRIP);
    glColor4fv(ui_colors[PROGRESSBAR_FILLED_COLOR]);
    glVertex2f(x, windowH - y);
    glVertex2f(x, windowH - y -h);
    glVertex2f(x+progress*w, windowH - y);
    glVertex2f(x+progress*w, windowH - y-h);
    
    glColor4fv(ui_colors[PROGRESSBAR_EMPTY_COLOR]);
    glVertex2f(x+progress*w, windowH - y);
    glVertex2f(x+progress*w, windowH - y -h);
    glVertex2f(x+w, windowH - y);
    glVertex2f(x+w, windowH -y -h);
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
    DrawPlainText(x, windowH-y+18.f, ui_colors[PROGRESSBAR_TITLE_COLOR], title);
    
    char buffer[16];
    sprintf(buffer, "%1.2lf%%", progress*100.0);
    GLfloat len = PlainTextLength(buffer);
    DrawPlainText(x+w-len, windowH- y + 18.f, ui_colors[PROGRESSBAR_VALUE_COLOR], buffer);
}

bool IMGUI::DoCheckBox(ui_id ID, GLfloat x, GLfloat y, bool value, const char* title)
{
    bool result = value;
    GLfloat size = 12.f;
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m); //12, 13, 14 -> x,y,z
    
    if(MouseInRect(x+m[12], y+m[13], size, size))
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
    glDisable(GL_TEXTURE_2D);
    
    if(isActive(ID))
        glColor4fv(ui_colors[CHECKBOX_ACTIVE_COLOR]);
    else if(isHot(ID))
        glColor4fv(ui_colors[CHECKBOX_HOT_COLOR]);
    else
        glColor4fv(ui_colors[CHECKBOX_NORMAL_COLOR]);
    
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(x, windowH - y);
    glVertex2f(x, windowH - y - size);
    glVertex2f(x+size, windowH - y);
    glVertex2f(x+size, windowH - y - size);
    glEnd();
    
    glColor4fv(ui_colors[CHECKBOX_BORDER_COLOR]);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, windowH - y);
    glVertex2f(x+size+1.f, windowH - y);
    glVertex2f(x+size+1.f, windowH - y - size -1.f);
    glVertex2f(x, windowH - y -size -1.f);
    glVertex2f(x, windowH - y +1.f);
    glEnd();
    
    if(value)
    {
        glColor4fv(ui_colors[CHECKBOX_TICK_COLOR]);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(x+0.2f*size, windowH - y - 0.2f*size);
        glVertex2f(x+0.2f*size, windowH - y - 0.8f*size);
        glVertex2f(x+0.8f*size, windowH - y - 0.2f*size);
        glVertex2f(x+0.8f*size, windowH - y - 0.8f*size);
        glEnd();
    }
    
    glEnable(GL_TEXTURE_2D);
    DrawPlainText(x+size+5.f, windowH - y, ui_colors[CHECKBOX_TITLE_COLOR], title);
    
    return result;
}

bool IMGUI::DoRadioButton(ui_id ID, GLfloat x, GLfloat y, bool value, const char* title)
{
    bool result = value;
    GLfloat size = 12.f;
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m); //12, 13, 14 -> x,y,z
    
    if(MouseInRect(x+m[12], y+m[13], size, size))
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
    glDisable(GL_TEXTURE_2D);
    
    if(isActive(ID))
        glColor4fv(ui_colors[RADIOBUTTON_ACTIVE_COLOR]);
    else if(isHot(ID))
        glColor4fv(ui_colors[RADIOBUTTON_HOT_COLOR]);
    else
        glColor4fv(ui_colors[RADIOBUTTON_NORMAL_COLOR]);
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x+size/2.0f, windowH - y - size/2.0f);
    for(int i=0; i<=12; i++)
        glVertex2f(x+size/2.f+size/2.f*sinf((float)i/12.f*M_PI*2.f), windowH - (y+size/2.0f+size/2.f*cosf((float)i/12.f*M_PI*2.f)));
    glEnd();
    
    glColor4fv(ui_colors[RADIOBUTTON_BORDER_COLOR]);
    glBegin(GL_LINE_STRIP);
    for(int i=0; i<=12; i++)
        glVertex2f(x+size/2.f+(size+2.f)/2.f*sinf((float)i/12.f*M_PI*2.f), windowH - (y+size/2.0f+(size+2.f)/2.f*cosf((float)i/12.f*M_PI*2.f)));
    glEnd();
    
    if(value)
    {
        glColor4fv(ui_colors[RADIOBUTTON_BUTTON_COLOR]);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x+size/2.0f, windowH - y - size/2.0f);
        for(int i=0; i<=12; i++)
            glVertex2f(x+size/2.f+(size*0.8f)/2.f*sinf((float)i/12.f*M_PI*2.f), windowH - (y+size/2.0f+(size*0.8f)/2.f*cosf((float)i/12.f*M_PI*2.f)));
        glEnd();
    }
    
    glEnable(GL_TEXTURE_2D);
    DrawPlainText(x+size+5.f, windowH - y, ui_colors[RADIOBUTTON_TITLE_COLOR], title);
    
    return result;
}

bool IMGUI::DoTimePlot(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, Sensor* sens, std::vector<unsigned short>& dims, const char* title, double fixedRange[2], unsigned int historyLength)
{
    bool result = false;
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m); //12, 13, 14 -> x,y,z
    
    if(MouseInRect(x+m[12], y+m[13], w, h))
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
    glDisable(GL_TEXTURE_2D);
    
    /*if(UI->isActive(ID))
        glColor4fv(ui_colors[BUTTON_ACTIVE_COLOR]);
    else if(UI->isHot(ID))
        glColor4fv(ui_colors[BUTTON_HOT_COLOR]);
    else
        glColor4fv(ui_colors[BUTTON_NORMAL_COLOR]);
    */
    
    //background
    glColor4fv(ui_colors[PLOT_BACKGROUND_COLOR]);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(x, windowH - y);
    glVertex2f(x, windowH - y -h);
    glVertex2f(x+w, windowH - y);
    glVertex2f(x+w, windowH - y -h);
    glEnd();
    
    //data
    const std::deque<Sample*>& data = sens->getHistory();
    if(data.size() > 1)
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
        //glDisable(GL_LINE_SMOOTH);
        for(int n = 0; n < dims.size(); n++)
        {
            glColor4fv(ui_colors[PLOT_DATA_COLOR1 + n % 6]);
            
            //graph
            glBegin(GL_LINE_STRIP);
            for(int i = 0;  i < data.size(); i++)
            {
                btScalar value = data[i]->getValue(dims[n]);
                glVertex2f(x + dt*i, windowH - y - (h-10.f) + (value-minValue)*dy);
            }
            glEnd();
            
            //legend
            glBegin(GL_TRIANGLE_STRIP);
            /*glVertex2f(x-3.f, windowH - y  - ((GLfloat)n/(GLfloat)(dims.size())) * h);
            glVertex2f(x, windowH - y  - ((GLfloat)n/(GLfloat)(dims.size())) * h);
            glVertex2f(x-3.f, windowH - y - ((GLfloat)(n+1)/(GLfloat)(dims.size())) * h);
            glVertex2f(x, windowH - y - ((GLfloat)(n+1)/(GLfloat)(dims.size())) * h);*/
            
            glVertex2f(x-10.f, windowH - y  - n * 10.f);
            glVertex2f(x, windowH - y - n * 10.f);
            glVertex2f(x-10.f, windowH - y - (n+1) * 10.f);
            glVertex2f(x, windowH - y - (n+1) * 10.f);
            
            glEnd();
        }
        
        glColor4fv(ui_colors[PLOT_AXES_COLOR]);
        glBegin(GL_LINES);
        if(minValue < 0 && maxValue > 0)
        {
            glVertex2f(x, windowH - y - (h-10.f) - minValue * dy);
            glVertex2f(x + w, windowH - y - (h-10.f) - minValue * dy);
        }
        glEnd();
        
        //glEnable(GL_LINE_SMOOTH);
        glEnable(GL_TEXTURE_2D);
        char values[32];
        sprintf(values, "%1.6f %1.6f", minValue, maxValue);
        DrawPlainText(x, windowH - y - h + 10.f, ui_colors[PLOT_TITLE_COLOR], values);
    }
        
    //title
    glEnable(GL_TEXTURE_2D);
    GLfloat len = PlainTextLength(title);
	
    DrawPlainText(x+floorf((w-len)/2.f), windowH - y - 10.f, ui_colors[PLOT_TITLE_COLOR], title);
    
    return result;
}
