//
//  IMGUI.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/27/12.
//  Copyright (c) 2012-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_IMGUI__
#define __Stonefish_IMGUI__

#include <SDL2/SDL_keyboard.h>
#include "StonefishCommon.h"
#include "graphics/OpenGLDataStructs.h"

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
#define CORNER_RADIUS 5.f

namespace sf
{
    //! A structure holiding widget data.
    struct ui_id
    {
        int owner;
        int item;
        int index;
    };
    
    class GLSLShader;
    class OpenGLPrinter;
    class ScalarSensor;
    
    //! A class implementing an immidiate mode graphical used interface.
    class IMGUI
    {
    public:
        //! A constructor.
        /*!
         \param windowWidth the width of the program window
         \param windowHeight the height of the program window
         \param hue the hue used to generate colors of the interface
         */
        IMGUI(GLint windowWidth, GLint windowHeight, GLfloat hue = 0.52f);
        
        //! A destructor.
        ~IMGUI();
        
        //! A method used to resize the interface.
        /*!
         \param windowWidth the new width of the program window
         \param windowHeight the new height of the program window
         */
        void Resize(GLint windowWidth, GLint windowHeight);
        
        //! A method that generates blurred interface background.
        void GenerateBackground();
        
        //! A method that starts rendering the GUI.
        void Begin();
        
        //! A method that finishes rendering the GUI.
        void End();
        
        //! A method that informs if any widget is currently active.
        bool isAnyActive();
        
        //! A method returning the width of the GUI.
        int getWindowHeight();
        
        //! A method returning the height of the GUI.
        int getWindowWidth();
        
        //! A method returning the id of a background texture.
        GLuint getTranslucentTexture();
        
        //! A method servicing the mouse down event.
        /*!
         \param x coordinate
         \param y coordinate
         \param leftButton a flag designating if left button was pressed
         */
        void MouseDown(int x, int y, bool leftButton);
        
        //! A method servicing the mouse up event.
        /*!
         \param x coordinate
         \param y coordinate
         \param leftButton a flag designating if left button was released
         */
        void MouseUp(int x, int y, bool leftButton);
        
        //! A method servicing the mouse move event.
        /*!
         \param x coordinate
         \param y coordinate
         */
        void MouseMove(int x, int y);
        
        //! A method servicing the keyboard key down event.
        /*!
         \param key the code of the pressed key
         */
        void KeyDown(SDL_Keycode key);
        
        //! A method servicing the keyboard key up event.
        /*!
         \param key the code of the released key
         */
        void KeyUp(SDL_Keycode key);
        
        //! A method used to create a panel widget.
        /*!
         \param x the x coordinate of the panel origin
         \param y the y coordinate of the panel origin
         \param w the width of the panel
         \param h the height of the panel
         */
        void DoPanel(GLfloat x, GLfloat y, GLfloat w, GLfloat h);
        
        //! A method used to create a label widget.
        /*!
         \param x the x coordinate of the label origin
         \param y the y coordinate of the label origin
         \param text a pointer to the string
         \param color a pointer to a color
         */
        void DoLabel(GLfloat x, GLfloat y, const char* text, GLfloat* color = NULL);
        
        //! A method used to create a progress bar widget.
        /*!
         \param x the x coordinate of the progress bar origin
         \param y the y coordinate of the progress bar origin
         \param w the width of the progress bar
         \param progress the value of the progress
         \param title a pointer to the title
         */
        void DoProgressBar(GLfloat x, GLfloat y, GLfloat w, Scalar progress, const char* title);
        
        //! A method used to create the button widget.
        /*!
         \param ui_id an id structure
         \param x the x coordinate of the button origin
         \param y the y coordinate of the button origin
         \param w the width of the button
         \param h the height of the button
         \param title a pointer to the button text
         \return button status
         */
        bool DoButton(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, const char* title);
        
        //! A method used to create a slider widget.
        /*!
         \param ui_id an id structure
         \param x the x coordinate of the slider origin
         \param y the y coordinate of the slider origin
         \param w the width of the slider
         \param min the minimum value of the slider
         \param max the maximum value of the slider
         \param value the current value of the slider
         \param title a pointer to the slider title
         \return new value
         */
        Scalar DoSlider(ui_id ID, GLfloat x, GLfloat y, GLfloat w, Scalar min, Scalar max, Scalar value, const char* title);
        
        //! A method used to create a checkbox widget.
        /*!
         \param ui_id an id structure
         \param x the x coordinate of the checkbox origin
         \param y the y coordinate of the checkbox origin
         \param w the width of the checkbox
         \param value the current value of the checkbox
         \param title a pointer to the checkbox title
         \return new checkbox status
         */
        bool DoCheckBox(ui_id ID, GLfloat x, GLfloat y, GLfloat w, bool value, const char* title);
        
        //! A method used to create a time plot widget.
        /*!
         \param ui_id an id structure
         \param x the x coordinate of the plot origin
         \param y the y coordinate of the plot origin
         \param w the width of the plot
         \param h the height of the plot
         \param sens a pointer to the sensor used as source of data
         \param dims a list of dimensions selected from all sensor dimensions
         \param title a pointer to the plot title
         \param fixedRange a range of the Y axis (if NULL the plot adjusts automatically)
         */
        bool DoTimePlot(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, ScalarSensor* sens, std::vector<unsigned short>& dims, const char* title, Scalar fixedRange[2] = NULL);
        
        //! A method used to create a XY plot widget.
        /*!
         \param ui_id an id structure
         \param x the x coordinate of the plot origin
         \param y the y coordinate of the plot origin
         \param w the width of the plot
         \param h the height of the plot
         \param sensX a pointer to the sensor used as source of data for X axis
         \param dimX the id of the sensor dimension used as data source
         \param sensY a pointer to the sensor used as source of data for Y axis
         \param dimY the id of the sensor dimension used as data source
         \param title a pointer to the XY plot title
         */
        bool DoXYPlot(ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, ScalarSensor* sensX, unsigned short dimX, ScalarSensor* sensY, unsigned short dimY, const char* title);
        
    private:
        ui_id getHot();
        ui_id getActive();
        void setHot(ui_id newHot);
        void setActive(ui_id newActive);
        bool isHot(ui_id ID);
        bool isActive(ui_id ID);
        void clearActive();
        void clearHot();
        bool MouseInRect(int x, int y, int w, int h);
        bool MouseIsDown(bool leftButton);
        int getMouseX();
        int getMouseY();
        
        void DrawPlainText(GLfloat x, GLfloat y, glm::vec4 color, const char* text);
        GLfloat PlainTextLength(const char* text);
        glm::vec2 PlainTextDimensions(const char* text);
        void DrawRoundedRect(GLfloat x, GLfloat y, GLfloat w, GLfloat h, glm::vec4 color = glm::vec4(1));
        void DrawRect(GLfloat x, GLfloat y, GLfloat w, GLfloat h, glm::vec4 color = glm::vec4(1));
        
        GLint windowW,windowH;
        bool shaders;
        int mouseX, mouseY;
        bool mouseLeftDown, mouseRightDown;
        ui_id hot;
        ui_id active;
        
        OpenGLPrinter* plainPrinter;
        GLuint logoTexture;
        GLuint guiTexture;
        glm::vec4 theme[11];
        GLfloat backgroundMargin;
        
        //Translucent background
        GLuint guiVAO;
        GLuint translucentFBO;
        GLuint translucentTexture[2];
        GLSLShader* downsampleShader;
        GLSLShader* gaussianShader;
        GLSLShader* guiShader[2];
    };
}

#endif
