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
//  IMGUI.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/27/12.
//  Copyright (c) 2012-2022 Patryk Cieslak. All rights reserved.
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
    //! A class holiding widget id.
    class Uid
    {
    public:
        int owner;
        int item;
        
    private:
        int index = 0;
        friend class IMGUI;
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
         \param text the string
         \param color the color of the text
         \param scale the font scaling factor
         */
        void DoLabel(GLfloat x, GLfloat y, const std::string& text, glm::vec4 color = glm::vec4(-1.f), GLfloat scale = 1.f);
        
        //! A method used to create a progress bar widget.
        /*!
         \param x the x coordinate of the progress bar origin
         \param y the y coordinate of the progress bar origin
         \param w the width of the progress bar
         \param progress the value of the progress
         \param title the title string
         */
        void DoProgressBar(GLfloat x, GLfloat y, GLfloat w, Scalar progress, const std::string& title);
        
        //! A method used to create the button widget.
        /*!
         \param id a Uid structure
         \param x the x coordinate of the button origin
         \param y the y coordinate of the button origin
         \param w the width of the button
         \param h the height of the button
         \param title the title string
         \return button status
         */
        bool DoButton(Uid id, GLfloat x, GLfloat y, GLfloat w, GLfloat h, const std::string& title);
        
        //! A method used to create a slider widget.
        /*!
         \param id an Uid structure
         \param x the x coordinate of the slider origin
         \param y the y coordinate of the slider origin
         \param w the width of the slider
         \param min the minimum value of the slider
         \param max the maximum value of the slider
         \param value the current value of the slider
         \param title the title string
         \param decimalPlaces a display precision of the current slider value 
         \return new value
         */
        Scalar DoSlider(Uid id, GLfloat x, GLfloat y, GLfloat w, Scalar min, Scalar max, Scalar value, const std::string& title, unsigned int decimalPlaces = 2);
        
        //! A method used to create a checkbox widget.
        /*!
         \param id a Uid structure
         \param x the x coordinate of the checkbox origin
         \param y the y coordinate of the checkbox origin
         \param w the width of the checkbox
         \param value the current value of the checkbox
         \param title the title string
         \return new checkbox status
         */
        bool DoCheckBox(Uid id, GLfloat x, GLfloat y, GLfloat w, bool value, const std::string& title);
        
        //! A method used to create a combo box.
        /*!
         \param id a Uid structure
         \param x the x coordinate of the combobox origin
         \param y the y coordinate of the combobox origin
         \param w the width of the combobox
         \param options a list of items of the combobox
         \param value the current value of the combobox
         \param title the title string
         \return new combobox value
         */
        unsigned int DoComboBox(Uid id, GLfloat x, GLfloat y, GLfloat w, const std::vector<std::string>& options, unsigned int value, const std::string& title);
        
        //! A method used to create a time plot widget for a sensor.
        /*!
         \param id a Uid structure
         \param x the x coordinate of the plot origin
         \param y the y coordinate of the plot origin
         \param w the width of the plot
         \param h the height of the plot
         \param sens a pointer to the sensor used as source of data
         \param dims a list of dimensions selected from all sensor dimensions
         \param title the title string
         \param fixedRange a range of the Y axis (if NULL the plot adjusts automatically)
         */
        bool DoTimePlot(Uid id, GLfloat x, GLfloat y, GLfloat w, GLfloat h, ScalarSensor* sens, std::vector<unsigned short>& dims, const std::string& title, Scalar fixedRange[2] = NULL);
        
        //! A method used to create a time plot widget for and arbitrary array od data.
        /*!
         \param id a Uid structure
         \param x the x coordinate of the plot origin
         \param y the y coordinate of the plot origin
         \param w the width of the plot
         \param h the height of the plot
         \param data a reference to the vector of vectors of data points
         \param title the title string
         \param fixedRange a range of the Y axis (if NULL the plot adjusts automatically)
         */        
        bool DoTimePlot(Uid id, GLfloat x, GLfloat y, GLfloat w, GLfloat h, std::vector<std::vector<GLfloat> >& data, const std::string& title, Scalar fixedRange[2] = NULL);

        //! A method used to create a XY plot widget.
        /*!
         \param id a Uid structure
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
        bool DoXYPlot(Uid id, GLfloat x, GLfloat y, GLfloat w, GLfloat h, ScalarSensor* sensX, unsigned short dimX, ScalarSensor* sensY, unsigned short dimY, const std::string& title);
        void DrawRoundedRect(GLfloat x, GLfloat y, GLfloat w, GLfloat h, glm::vec4 color = glm::vec4(1));
 
    private:
        Uid getHot();
        Uid getActive();
        void setHot(Uid newHot);
        void setActive(Uid newActive);
        bool isHot(Uid id);
        bool isActive(Uid id);
        void clearActive();
        void clearHot();
        bool MouseInRect(int x, int y, int w, int h);
        bool MouseIsDown(bool leftButton);
        int getMouseX();
        int getMouseY();
        
        void DrawPlainText(GLfloat x, GLfloat y, glm::vec4 color, const std::string& text, GLfloat scale = 1.f);
        GLfloat PlainTextLength(const std::string& text);
        glm::vec2 PlainTextDimensions(const std::string& text);
        void DrawRect(GLfloat x, GLfloat y, GLfloat w, GLfloat h, glm::vec4 color = glm::vec4(1));
        void DrawArrow(GLfloat x, GLfloat y, GLfloat h, bool up, glm::vec4 color = glm::vec4(1));
        
        GLint windowW,windowH;
        bool shaders;
        int mouseX, mouseY;
        bool mouseLeftDown, mouseRightDown;
        Uid hot;
        Uid active;
        
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
