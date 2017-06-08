//
//  IMGUI.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/27/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_IMGUI__
#define __Stonefish_IMGUI__

#include "common.h"
#include <SDL2/SDL_keyboard.h>
#include "OpenGLPipeline.h"
#include "OpenGLPrinter.h"
#include "GLSLShader.h"
#include "SimpleSensor.h"

//interface colors
#define PANEL_COLOR 0                 //Panel/background
#define ACTIVE_TEXT_COLOR 1           //Label, title....
#define INACTIVE_TEXT_COLOR 2
#define ACTIVE_CONTROL_COLOR 3        //Button, slider...
#define INACTIVE_CONTROL_COLOR 4
#define HOT_CONTROL_COLOR 5
#define PUSHED_CONTROL_COLOR 6
#define EMPTY_COLOR 7                 //Slider, progress, checkbox, radio...
#define FILLED_COLOR 8
#define PLOT_COLOR 9                  //Plots
#define PLOT_TEXT_COLOR 10
#define GAUGE_SAFE_COLOR 11           //Gauge
#define GAUGE_DANGER_COLOR 12

struct ui_id
{
    int owner;
    int item;
    int index;
};

//singleton
class IMGUI
{
public:
    void Init(GLint windowWidth, GLint windowHeight, GLfloat hue = 0.52f);
    void GenerateBackground();
    void Begin();
    void End();
    bool MouseInRect(int x, int y, int w, int h);
    bool MouseIsDown(bool leftButton);
    
    ui_id getHot();
    ui_id getActive();
    void setHot(ui_id newHot);
    void setActive(ui_id newActive);
    bool isHot(ui_id ID);
    bool isActive(ui_id ID);
    bool isAnyActive();
    void clearActive();
    void clearHot();
    int getWindowHeight();
    int getWindowWidth();
    int getMouseX();
    int getMouseY();
    GLuint getTranslucentTexture();
    
    //input handling
    void MouseDown(int x, int y, bool leftButton);
    void MouseUp(int x, int y, bool leftButton);
    void MouseMove(int x, int y);
    void KeyDown(SDL_Keycode key);
    void KeyUp(SDL_Keycode key);
    
    //widgets
    //passive
    void DoPanel(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat radius = 0.f);
    void DoLabel(GLfloat x, GLfloat y, const char* text, GLfloat* color = NULL);
    void DoProgressBar(GLfloat x, GLfloat y, GLfloat barW, GLfloat barH, btScalar progress, const char* title);
    void DoGauge(GLfloat x, GLfloat y, GLfloat diameter, btScalar value, btScalar range[2], btScalar safeRange[2], const char* title);
    //active
    bool DoButton(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, const char* title);
    btScalar DoSlider(ui_id ID, GLfloat x, GLfloat y, GLfloat railW, GLfloat railH, GLfloat sliderW, GLfloat sliderH, btScalar min, btScalar max, btScalar value, const char* title);
    bool DoCheckBox(ui_id ID, GLfloat x, GLfloat y, bool value, const char* title);
    unsigned short DoRadioGroup(ui_id ID, GLfloat x, GLfloat y, unsigned short selection, std::vector<std::string>& items, const char* title);
    bool DoTimePlot(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, SimpleSensor* sens, std::vector<unsigned short>& dims, const char* title, bool plottingEnabled = true, btScalar fixedRange[2] = NULL, unsigned int historyLength = 0);
    bool DoXYPlot(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, SimpleSensor* sensX, unsigned short dimX, SimpleSensor* sensY, unsigned short dimY, const char* title, unsigned int historyLength = 0);
    
    static IMGUI* getInstance();
    static glm::vec4 HSV2RGB(glm::vec4 hsv);
    
private:
    IMGUI();
    ~IMGUI();
    void DrawPlainText(GLfloat x, GLfloat y, glm::vec4 color, const char* text);
    GLfloat PlainTextLength(const char* text);
    glm::vec2 PlainTextDimensions(const char* text);
    void DrawRoundedRect(GLfloat x, GLfloat y, GLfloat width, GLfloat height, GLfloat radius);
    
    GLint windowW,windowH;
    bool shaders;
    int mouseX, mouseY;
    bool mouseLeftDown, mouseRightDown;
    ui_id hot;
    ui_id active;
    
    OpenGLPrinter* plainPrinter;
    GLuint logoTexture;
    glm::vec4 theme[13];
    GLfloat backgroundMargin;
    
    //Translucent background
    GLuint translucentFBO;
    GLuint translucentTexture[2];
    GLSLShader* downsampleShader;
    GLSLShader* gaussianShader;
    
    static IMGUI* instance;
};

#endif
