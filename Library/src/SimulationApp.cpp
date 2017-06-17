//
//  SimulationApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "SimulationApp.h"
#include "SystemUtil.hpp"

SimulationApp::SimulationApp(const char* name, int width, int height, SimulationManager* sim)
{
    SimulationApp::handle = this;
	appName = name;
    winWidth = width;
    winHeight = height;
    finished = false;
    running = false;
    simulation = sim;
    simSpeedFactor = 1;
    lastPicked = NULL;
    loading = true;
    displayHUD = true;
    displayConsole = false;
    joystick = NULL;
    joystickAxes = NULL;
    joystickButtons = NULL;
    joystickHats = NULL;
    
	fps = 0.0;
	physics = 0.0;
}

SimulationApp::~SimulationApp()
{
	if(joystick != NULL)
	{
		delete [] joystickButtons;
		delete [] joystickAxes;
		delete [] joystickHats;
	}
}

void SimulationApp::setSimulationSpeed(btScalar factor)
{
    if(factor > 0.f)
        simSpeedFactor = factor;
    else
        simSpeedFactor = 1;
}

void SimulationApp::ShowHUD()
{
    displayHUD = true;
}

void SimulationApp::HideHUD()
{
    displayHUD = false;
}

void SimulationApp::ShowConsole()
{
    displayConsole = true;
}

void SimulationApp::HideConsole()
{
    displayConsole = false;
}

btScalar SimulationApp::getSimulationSpeed()
{
    return simSpeedFactor;
}

SimulationManager* SimulationApp::getSimulationManager()
{
    return simulation;
}

SDL_Joystick* SimulationApp::getJoystick()
{
    return joystick;
}

double SimulationApp::getFPS()
{
    return fps;
}

double SimulationApp::getPhysicsTime()
{
    return physics;
}

int SimulationApp::getWindowWidth()
{
    return winWidth;
}

int SimulationApp::getWindowHeight()
{
    return winHeight;
}

bool SimulationApp::isRunning()
{
	return running;
}

bool SimulationApp::isLoading()
{
    return loading;
}

const char* SimulationApp::getDataPath()
{
    return dataPath;
}

const char* SimulationApp::getShaderPath()
{
    return shaderPath;
}

void SimulationApp::Init(const char* dataPath, const char* shaderPath)
{
    this->dataPath = dataPath;
    this->shaderPath = shaderPath;
    
    //Basics
    InitializeSDL(); //Window initialization + loading thread
    
    //Continue initialization with console visible
    cInfo("Initializing rendering pipeline:");
    cInfo("Loading GUI...");
    IMGUI::getInstance()->Init(winWidth, winHeight);
    ShowHUD();
    OpenGLPipeline::getInstance()->Initialize(winWidth, winHeight);
    
    cInfo("Initializing simulation:");
    InitializeSimulation();
    
    cInfo("Running...");
    
    //Close loading console - exit loading thread
    SDL_Delay(1000);
    loading = false;
    int status = 0;
    SDL_WaitThread(loadingThread, &status);
}

void SimulationApp::InitializeSDL()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    
    //Create OpenGL contexts
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	
	//Create window
    window = SDL_CreateWindow(appName,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              winWidth,
                              winHeight,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
                              );
  
	glLoadingContext = SDL_GL_CreateContext(window);
	if(glLoadingContext == NULL)
	{
		printf("SDL2: OpenGL context could not be created: %s\n", SDL_GetError());
		exit(1);
	}
	
	glMainContext = SDL_GL_CreateContext(window);
	if(glMainContext == NULL)
	{
		printf("SDL2: OpenGL context could not be created: %s\n", SDL_GetError());
		exit(1);
	}
	
    //Check OpenGL context version
    int glVersionMajor, glVersionMinor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glVersionMajor);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glVersionMinor);
	SDL_GL_SetSwapInterval(0);
    
	//Initialize basic drawing functions and console
    if(glewInit() != GLEW_OK) 
	{
		printf("Failed to initialize GLEW!\n");
		exit(1);
	}
	
	bool shaders = GLSLShader::Init();
	if(!shaders)
	{
		printf("No shader support!\n");
		exit(1);
	}
    
	Console::getInstance()->Init(winWidth, winHeight);
	cInfo("Window created. OpenGL %d.%d contexts created.", glVersionMajor, glVersionMinor);
	
    //Create loading thread
    LoadingThreadData* data = new LoadingThreadData();
    data->app = this;
    data->mutex = Console::getInstance()->getLinesMutex();
    loadingThread = SDL_CreateThread(SimulationApp::RenderLoadingScreen, "loadingThread", data);
	
    //Look for joysticks
    int jcount = SDL_NumJoysticks();
    
    if(jcount > 0)
    {
        joystick = SDL_JoystickOpen(0);
        joystickButtons = new bool[SDL_JoystickNumButtons(joystick)];
        memset(joystickButtons, 0, SDL_JoystickNumButtons(joystick));
        joystickAxes = new int16_t[SDL_JoystickNumAxes(joystick)];
        memset(joystickAxes, 0, SDL_JoystickNumAxes(joystick) * sizeof(int16_t));
        joystickHats = new uint8_t[SDL_JoystickNumHats(joystick)];
        memset(joystickHats, 0, SDL_JoystickNumHats(joystick));
        cInfo("Joystick %s connected (%d axes, %d hats, %d buttons)", SDL_JoystickName(joystick),
                                                                      SDL_JoystickNumAxes(joystick),
                                                                      SDL_JoystickNumHats(joystick),
                                                                      SDL_JoystickNumButtons(joystick));
    }
}

void SimulationApp::InitializeSimulation()
{
    cInfo("Building scenario...");
    simulation->RestartScenario();
    cInfo("Synchronizing motion states...");
    simulation->getDynamicsWorld()->synchronizeMotionStates();
    cInfo("Simulation initialized -> using Bullet Physics %d.%d.", btGetVersion()/100, btGetVersion()%100);
}

void SimulationApp::WindowEvent(SDL_Event* event)
{
    int w, h;
    
    switch(event->window.event)
    {
        case SDL_WINDOWEVENT_RESIZED:
            SDL_GetWindowSize(window, &w, &h);
            IMGUI::getInstance()->Init(w, h);
            break;
    }
}

void SimulationApp::KeyDown(SDL_Event *event)
{
    switch (event->key.keysym.sym)
    {
        case SDLK_ESCAPE:
            Quit();
            break;
            
        case SDLK_SPACE:
            lastPicked = NULL;
            getSimulationManager()->RestartScenario();
            StartSimulation();
            break;
            
        case SDLK_h:
            displayHUD = !displayHUD;
            break;
            
        case SDLK_c:
            displayConsole = !displayConsole;
            Console::getInstance()->ResetScroll();
            break;
            
        default:
            break;
    }
}

void SimulationApp::KeyUp(SDL_Event *event)
{
}

void SimulationApp::MouseDown(SDL_Event *event)
{
}

void SimulationApp::MouseUp(SDL_Event *event)
{
}

void SimulationApp::MouseMove(SDL_Event *event)
{
}

void SimulationApp::MouseScroll(SDL_Event *event)
{
}

void SimulationApp::JoystickDown(SDL_Event *event)
{
}

void SimulationApp::JoystickUp(SDL_Event *event)
{
}

void SimulationApp::ProcessInputs()
{
}

void SimulationApp::EventLoop()
{
    SDL_Event event;
    bool mouseWasDown = false;
    startTime = GetTimeInMicroseconds();
    
    while(!finished)
    {
        SDL_FlushEvents(SDL_FINGERDOWN, SDL_MULTIGESTURE);

        if(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_WINDOWEVENT:
                    WindowEvent(&event);
                    break;
                
                case SDL_TEXTINPUT:
                    break;
                    
                case SDL_KEYDOWN:
                {
                    IMGUI::getInstance()->KeyDown(event.key.keysym.sym);
                    KeyDown(&event);
                    break;
                }
                    
                case SDL_KEYUP:
                {
                    IMGUI::getInstance()->KeyUp(event.key.keysym.sym);
                    KeyUp(&event);
                    break;
                }
                    
                case SDL_MOUSEBUTTONDOWN:
                    IMGUI::getInstance()->MouseDown(event.button.x, event.button.y, event.button.button == SDL_BUTTON_LEFT);
                    mouseWasDown = true;
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    IMGUI::getInstance()->MouseUp(event.button.x, event.button.y, event.button.button == SDL_BUTTON_LEFT);
                    MouseUp(&event);
                    break;
                    
                case SDL_MOUSEMOTION:
                    IMGUI::getInstance()->MouseMove(event.motion.x, event.motion.y);
                    MouseMove(&event);
                    break;
                    
                case SDL_MOUSEWHEEL:
                    if(displayConsole)
                        Console::getInstance()->Scroll((GLfloat)-event.wheel.y);
                    else
                        MouseScroll(&event);
                    break;
                    
                case SDL_JOYBUTTONDOWN:
                    joystickButtons[event.jbutton.button] = true;
                    JoystickDown(&event);
                    break;
                    
                case SDL_JOYBUTTONUP:
                    joystickButtons[event.jbutton.button] = false;
                    JoystickUp(&event);
                    break;
                    
                case SDL_QUIT:
                    finished = true;
                    break;
            }
        }

        if(joystick != NULL)
        {
            for(int i=0; i<SDL_JoystickNumAxes(joystick); i++)
                joystickAxes[i] = SDL_JoystickGetAxis(joystick, i);
        
            for(int i=0; i<SDL_JoystickNumHats(joystick); i++)
                joystickHats[i] = SDL_JoystickGetHat(joystick, i);
        }
        
        ProcessInputs();
        AppLoop();
        
        //workaround for checking if IMGUI is being manipulated
        if(mouseWasDown && !IMGUI::getInstance()->isAnyActive())
        {
            lastPicked = simulation->PickEntity(event.button.x, event.button.y);
            MouseDown(&event);
        }
        mouseWasDown = false;
    }
}

void SimulationApp::AppLoop()
{
    //FPS update
    uint64_t endTime = GetTimeInMicroseconds();
    double eleapsed = (endTime - startTime)/1000000.0;
    fps = 1.0/eleapsed;
    startTime = endTime;
    
    //Simulation
    if(running)
    {
        simulation->AdvanceSimulation(GetTimeInMicroseconds()*simSpeedFactor);
        physics = simulation->getPhysicsTimeInMiliseconds();
    }
    
    //Rendering
    OpenGLPipeline::getInstance()->Render(simulation);
    OpenGLPipeline::getInstance()->DrawDisplay();
    
    //GUI & Console
    if(displayConsole)
    {
        IMGUI::getInstance()->GenerateBackground();
        Console::getInstance()->Render(true, eleapsed);
    }
    else
    {
        if(displayHUD)
        {
            IMGUI::getInstance()->GenerateBackground();
            IMGUI::getInstance()->Begin();
            DoHUD();
            IMGUI::getInstance()->End();
        }
        else
        {
            char buffer[16];
            double fps = getFPS();
            sprintf(buffer, "FPS: %1.2lf", fps);
            
            IMGUI::getInstance()->Begin();
            IMGUI::getInstance()->DoLabel(10, 10, buffer);
            IMGUI::getInstance()->End();
        }
    }
    
	SDL_GL_SwapWindow(window);
}

void SimulationApp::DoHUD()
{
    char buffer[256];
    
    ui_id slider1;
    slider1.owner = 0;
    slider1.item = 3;
    slider1.index = 0;
    getSimulationManager()->setStepsPerSecond(IMGUI::getInstance()->DoSlider(slider1, 5.f, 5.f, 120.f, 100.0, 5000.0, getSimulationManager()->getStepsPerSecond(), "Steps/s"));
    
    //Bottom panel
    IMGUI::getInstance()->DoPanel(0, getWindowHeight()-30.f, getWindowWidth(), 30.f);
    
	double fps = getFPS();
    sprintf(buffer, "FPS: %1.2lf", fps);
    IMGUI::getInstance()->DoLabel(10, getWindowHeight() - 20.f, buffer);
    
    sprintf(buffer, "Physics time: %1.1lf%% (%1.2lf ms)", getPhysicsTime()/(10.0/fps), getPhysicsTime());
    IMGUI::getInstance()->DoLabel(100, getWindowHeight() - 20.f, buffer);
    
    sprintf(buffer, "Simulation speed: %1.2fx", getSimulationSpeed());
    IMGUI::getInstance()->DoLabel(300, getWindowHeight() - 20.f, buffer);
    
    sprintf(buffer, "Simulation time: %1.2f s", getSimulationManager()->getSimulationTime());
    IMGUI::getInstance()->DoLabel(470, getWindowHeight() - 20.f, buffer);
    
    if(lastPicked != NULL)
    {
        sprintf(buffer, "Last picked entity: %s", lastPicked->getName().c_str());
        IMGUI::getInstance()->DoLabel(630, getWindowHeight() - 20.f, buffer);
    }
}

void SimulationApp::StartSimulation()
{
    simulation->StartSimulation();
    running = true;
}

void SimulationApp::ResumeSimulation()
{
    simulation->ResumeSimulation();
    running = true;
}

void SimulationApp::StopSimulation()
{
    running = false;
    physics = 0.f;
    simulation->StopSimulation();
}

void SimulationApp::Quit()
{
    finished = true;
}

void SimulationApp::CleanUp()
{
    if(simulation != NULL)
        delete simulation;
    
    if(joystick != NULL)
        SDL_JoystickClose(0);
    
    SDL_GL_DeleteContext(glLoadingContext);
    SDL_GL_DeleteContext(glMainContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

SimulationApp* SimulationApp::handle = NULL;

SimulationApp* SimulationApp::getApp()
{
    return SimulationApp::handle;
}

int SimulationApp::RenderLoadingScreen(void* data)
{
    //Get application
    LoadingThreadData* ltdata = (LoadingThreadData*)data;
    
    //Make drawing in this thread possible
    SDL_GL_MakeCurrent(ltdata->app->window, ltdata->app->glLoadingContext);  
    
    //Render loading screen
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	
    while(ltdata->app->loading)
    {
		glClear(GL_COLOR_BUFFER_BIT);
        
        //Lock to prevent adding lines to the console while rendering
        SDL_LockMutex(ltdata->mutex);
        Console::getInstance()->Render(false, 0);
        SDL_UnlockMutex(ltdata->mutex);
        
        glFlush();
        glFinish();
        SDL_GL_SwapWindow(ltdata->app->window);
    }
	
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
    
    //Detach thread from GL context
    SDL_GL_MakeCurrent(ltdata->app->window, NULL);
    
    return 0;
}


