//
//  SimulationApp.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SimulationApp__
#define __Stonefish_SimulationApp__

#include "common.h"
#include <SDL2/SDL.h>
#include "OpenGLPipeline.h"
#include "IMGUI.h"
#include "Console.h"
#include "SimulationManager.h"

/*!
    @class SimulationApp
    A class that defines an application interface combined with a simulation manager.
 */
class SimulationApp
{
public:
    SimulationApp(std::string name, int width, int height, SimulationManager* sim);
    virtual ~SimulationApp();
    
    void Init(std::string dataPath, std::string shaderPath);
    void CleanUp();
    void Quit();
    
    void EventLoop();
    void AppLoop();
    
    void StartSimulation();
    void ResumeSimulation();
    void StopSimulation();
    
    //virtual methods
    virtual void DoHUD();
    void ShowHUD();
    void HideHUD();
    void ShowConsole();
    void HideConsole();
    
    //input handling
    bool* joystickButtons;
    int16_t* joystickAxes;
    uint8_t* joystickHats;
    
    virtual void ProcessInputs();
    virtual void WindowEvent(SDL_Event* event);
    virtual void KeyDown(SDL_Event* event);
    virtual void KeyUp(SDL_Event* event);
    virtual void MouseDown(SDL_Event* event);
    virtual void MouseUp(SDL_Event* event);
    virtual void MouseMove(SDL_Event* event);
    virtual void MouseScroll(SDL_Event* event);
    virtual void JoystickDown(SDL_Event* event);
    virtual void JoystickUp(SDL_Event* event);
    
    //accessors
    SimulationManager* getSimulationManager();
    SDL_Joystick* getJoystick();
	double getDrawingTime();
    double getPhysicsTime();
    int getWindowWidth();
    int getWindowHeight();
	bool isRunning();
    bool isLoading();
	std::string getShaderPath();
    std::string getDataPath();

    //statics
    static SimulationApp* getApp();
    
private:
    void InitializeSDL();
    void InitializeSimulation();
    void InitializeGUI();
    
    SDL_GLContext glMainContext;
    SDL_GLContext glLoadingContext;
    SDL_Thread* loadingThread;
    SDL_Window* window;
    SDL_Joystick* joystick;
    
    SimulationManager* simulation;
    Entity* lastPicked;
    
    std::string appName;
    std::string shaderPath;
    std::string dataPath;
    int winWidth;
    int winHeight;
    bool finished;
    bool running;
    bool loading;
	double drawingTime;
    double physicsTime;
    uint64_t startTime;
    bool displayHUD;
    bool displayConsole;
  
    static int RenderLoadingScreen(void* app);
    static SimulationApp* handle;
};

typedef struct
{
    SimulationApp* app;
    SDL_mutex* mutex;
}
LoadingThreadData;

#endif
