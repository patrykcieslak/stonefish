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
//  GraphicalSimulationApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GraphicalSimulationApp__
#define __Stonefish_GraphicalSimulationApp__

#include "graphics/OpenGLDataStructs.h"
#include <SDL2/SDL.h>
#include "core/SimulationApp.h"

namespace sf
{
    class OpenGLConsole;
    class IMGUI;
    class OpenGLPipeline;
    class Entity;
    class MovingEntity;
    
    //! A class that implements an interface of a graphical application.
    class GraphicalSimulationApp : public SimulationApp
    {
    public:
        //! A constructor.
        /*!
         \param title a title for the application
         \param dataDirPath a path to the directory containing simulation data
         \param s a structure containing the rendering settings
         \param h a structure containing the helper objects display settings
         \param sim a pointer to the simulation manager
         */
        GraphicalSimulationApp(std::string title, std::string dataDirPath, RenderSettings s, HelperSettings h, SimulationManager* sim);
        
        //! A destructor.
        virtual ~GraphicalSimulationApp();

        //! A method that starts the simulation on demand.
        void StartSimulation() override;

        //! A method that stops the simulation on demand.
        void StopSimulation() override;

        //! A method that resumes the simulation on demand.
        void ResumeSimulation() override;

        //! A method that performs a single simulation step and necessary updates.
        void StepSimulation() override;
        
        //! A method implementing the GUI on top of the simulation window.
        virtual void DoHUD();
        
        //! A method to enable rendering the GUI.
        void ShowHUD();
        
        //! A method to disable rendering the GUI.
        void HideHUD();
        
        //! A method to enable showing the console.
        void ShowConsole();
        
        //! A method to disable showing the console.
        void HideConsole();
        
        //! A method implementing input processing.
        virtual void ProcessInputs();
        
        //! A method implementing window event handling.
        /*!
         \param event a pointer to the event info structure
         */
        virtual void WindowEvent(SDL_Event* event);
        
        //! A method implementing keyboard key down event handling.
        /*!
         \param event a pointer to the event info structure
         */
        virtual void KeyDown(SDL_Event* event);
        
        //! A method implementing keyboard key up event handling.
        /*!
         \param event a pointer to the event info structure
         */
        virtual void KeyUp(SDL_Event* event);
        
        //! A method implementing mouse down event handling.
        /*!
         \param event a pointer to the event info structure
         */
        virtual void MouseDown(SDL_Event* event);
        
        //! A method implementing mouse up event handling.
        /*!
         \param event a pointer to the event info structure
         */
        virtual void MouseUp(SDL_Event* event);
        
        //! A method implementing mouse move event handling.
        /*!
         \param event a pointer to the event info structure
         */
        virtual void MouseMove(SDL_Event* event);
        
        //! A method implementing mouse scroll event handling.
        /*!
         \param event a pointer to the event info structure
         */
        virtual void MouseScroll(SDL_Event* event);
        
        //! A method implementing joystick key down event handling.
        /*!
         \param event a pointer to the event info structure
         */
        virtual void JoystickDown(SDL_Event* event);
        
        //! A method implementing joystick key up event handling.
        /*!
         \param event a pointer to the event info structure
         */
        virtual void JoystickUp(SDL_Event* event);
        
        //! A method returning a pointer to the OpenGL pipeline.
        OpenGLPipeline* getGLPipeline();
        
        //! A method returning a pointer to the GUI.
        IMGUI* getGUI();

        //! A method returning a pointer to the selected entity.
        std::pair<Entity*, int> getSelectedEntity();
        
        //! A method informing if the application is graphical.
        bool hasGraphics();
        
        //! A method returning a pointer to the joystick info structure.
        SDL_Joystick* getJoystick();
        
        //! A method returning the graphics rendering time in seconds.
        /*!
          \param max return maximum drawing time?
          \return moving average drawing time or maximum drawing time [ms]
        */
        double getDrawingTime(bool max = false);
        
        //! A method returning the width of the window.
        int getWindowWidth();
        
        //! A method returning the height of the window.
        int getWindowHeight();
        
        //! A method informing if the simulation is in the loading stage.
        bool isLoading();
        
        //! A method returning the path to the shader directory.
        std::string getShaderPath();
        
        //! A method returning the current rendering settings.
        RenderSettings getRenderSettings() const;
        
        //! A method returning a mutable reference to the helper object rendering settings.
        HelperSettings& getHelperSettings();

        //! A method used to enable frame rate limitting.
        void setLimitFramerate(bool enabled);
        
    protected:
        void Init();
        void LoopInternal();
        void CleanUp();
        
        virtual void InitializeGUI();
        
    private:
        void InitializeSDL();
        void RenderLoop();
        
        SDL_GLContext glMainContext;
        SDL_GLContext glLoadingContext;
        SDL_Thread* loadingThread;
        SDL_Thread* simulationThread;
        SDL_Window* window;
        SDL_Joystick* joystick;
        bool* joystickButtons;
        int16_t* joystickAxes;
        uint8_t* joystickHats;
        SDL_Event mouseWasDown;
        
        IMGUI* gui;
        OpenGLPipeline* glPipeline;
        
        MovingEntity* trackballCenter;
        std::pair<Entity*, int> selectedEntity;
        bool displayHUD;
        bool displayKeymap;
        bool displayConsole;
        bool displayPerformance;
        std::string shaderPath;
        bool loading;
        double drawingTime;
        double maxDrawingTime;
        int maxCounter;
        int windowW;
        int windowH;
        RenderSettings rSettings;
        HelperSettings hSettings;
        GLuint timeQuery[2];
        GLint timeQueryPingpong;
        bool limitFramerate;

        static int RenderLoadingScreen(void* data);
        static int RunSimulation(void* data);
    };
    
    //! A structure used to pass information between threads.
    struct GraphicalSimulationThreadData
    {
        GraphicalSimulationApp& app;
    };
}
#endif
