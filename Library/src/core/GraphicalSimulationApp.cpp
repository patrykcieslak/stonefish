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
//  GraphicalSimulationApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2019 Patryk Cieslak. All rights reserved.
//

#include "core/GraphicalSimulationApp.h"

#include <chrono>
#include <thread>
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLConsole.h"
#include "graphics/IMGUI.h"
#include "graphics/OpenGLTrackball.h"
#include "utils/SystemUtil.hpp"
#include "entities/Entity.h"
#include "entities/StaticEntity.h"
#include "entities/SolidEntity.h"

namespace sf
{

GraphicalSimulationApp::GraphicalSimulationApp(std::string name, std::string dataDirPath, RenderSettings r, HelperSettings h, SimulationManager* sim)
: SimulationApp(name, dataDirPath, sim)
{
#ifdef SHADER_DIR_PATH
    shaderPath = SHADER_DIR_PATH;
#else
    shaderPath = "/usr/local/share/Stonefish/shaders/";
#endif
    glLoadingContext = NULL;
    glMainContext = NULL;
    lastPicked = NULL;
    displayHUD = true;
    displayConsole = false;
    joystick = NULL;
    joystickAxes = NULL;
    joystickButtons = NULL;
    joystickHats = NULL;
    simulationThread = NULL;
    loadingThread = NULL;
    glPipeline = NULL;
    gui = NULL;
	drawingTime = 0.0;
    windowW = r.windowW;
    windowH = r.windowH;
    rSettings = r;
    hSettings = h;
}

GraphicalSimulationApp::~GraphicalSimulationApp()
{
    if(console != NULL) delete console;
    if(glPipeline != NULL) delete glPipeline;
    if(gui != NULL) delete gui;
    
	if(joystick != NULL)
	{
		delete [] joystickButtons;
		delete [] joystickAxes;
		delete [] joystickHats;
	}
}

void GraphicalSimulationApp::ShowHUD()
{
    displayHUD = true;
}

void GraphicalSimulationApp::HideHUD()
{
    displayHUD = false;
}

void GraphicalSimulationApp::ShowConsole()
{
    displayConsole = true;
}

void GraphicalSimulationApp::HideConsole()
{
    displayConsole = false;
}

OpenGLPipeline* GraphicalSimulationApp::getGLPipeline()
{
    return glPipeline;
}

IMGUI* GraphicalSimulationApp::getGUI()
{
    return gui;
}

bool GraphicalSimulationApp::hasGraphics()
{
	return true;
}

SDL_Joystick* GraphicalSimulationApp::getJoystick()
{
    return joystick;
}

double GraphicalSimulationApp::getDrawingTime()
{
    return drawingTime;
}

int GraphicalSimulationApp::getWindowWidth()
{
    return windowW;
}

int GraphicalSimulationApp::getWindowHeight()
{
    return windowH;
}

std::string GraphicalSimulationApp::getShaderPath()
{
    return shaderPath;
}

RenderSettings GraphicalSimulationApp::getRenderSettings() const
{
    return glPipeline->getRenderSettings();
}

HelperSettings& GraphicalSimulationApp::getHelperSettings()
{
    return glPipeline->getHelperSettings();
}

void GraphicalSimulationApp::Init()
{
    //Basics
    loading = true;
    console = new Console(); //Create temporary text console
    InitializeSDL(); //Window initialization + loading thread
    
    //Continue initialization with console visible
    cInfo("Initializing rendering pipeline:");
    cInfo("Loading GUI...");
    gui = new IMGUI(windowW, windowH);
    InitializeGUI(); //Initialize non-standard graphical elements
    glPipeline = new OpenGLPipeline(rSettings, hSettings);
    ShowHUD();
    
    cInfo("Initializing simulation:");
    InitializeSimulation();
    
    cInfo("Ready for running...");
    SDL_Delay(1000);
    
    //Close loading console - exit loading thread
    loading = false;
    int status = 0;
    SDL_WaitThread(loadingThread, &status);
    SDL_GL_MakeCurrent(window, glMainContext);
}

void GraphicalSimulationApp::InitializeSDL()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK);
    
    //Create OpenGL contexts
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    
	//Create window
    window = SDL_CreateWindow(getName().c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              windowW,
                              windowH,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN// | SDL_WINDOW_ALLOW_HIGHDPI
                              );
  
	glLoadingContext = SDL_GL_CreateContext(window);
	if(glLoadingContext == NULL)
        cCritical("SDL2: %s", SDL_GetError());
    
	glMainContext = SDL_GL_CreateContext(window);
	if(glMainContext == NULL)
		cCritical("SDL2: %s", SDL_GetError());
	
    //Disable vertical synchronization --> use framerate limitting instead (e.g. max 60 FPS)
    if(SDL_GL_SetSwapInterval(0) == -1)
        cError("SDL2: %s", SDL_GetError());
    
    //Check OpenGL context version
    int glVersionMajor, glVersionMinor;
    glGetIntegerv(GL_MAJOR_VERSION, &glVersionMajor);
    glGetIntegerv(GL_MINOR_VERSION, &glVersionMinor);
    cInfo("Window created. OpenGL %d.%d contexts created.", glVersionMajor, glVersionMinor);
    
	//Initialize basic drawing functions and console
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
		cCritical("Failed to initialize GLEW!");
	
	if(!GLSLShader::Init())
		cCritical("No shader support!");
	
    std::vector<ConsoleMessage> textLines = console->getLines();
    delete console;
    console = new OpenGLConsole();
    for(size_t i=0; i<textLines.size(); ++i)
        console->AppendMessage(textLines[i]);
    ((OpenGLConsole*)console)->Init(windowW, windowH);
    
    //Create loading thread
    LoadingThreadData* data = new LoadingThreadData();
    data->app = this;
    data->mutex = console->getLinesMutex();
    loadingThread = SDL_CreateThread(GraphicalSimulationApp::RenderLoadingScreen, "loadingThread", data);
	
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

void GraphicalSimulationApp::InitializeGUI()
{
}

void GraphicalSimulationApp::WindowEvent(SDL_Event* event)
{
    int w, h;
    
    switch(event->window.event)
    {
        case SDL_WINDOWEVENT_RESIZED:
            SDL_GetWindowSize(window, &w, &h);
            gui->Resize(w, h);
            break;
    }
}

void GraphicalSimulationApp::KeyDown(SDL_Event *event)
{
    switch (event->key.keysym.sym)
    {
        case SDLK_ESCAPE:
            Quit();
            break;
            
        case SDLK_SPACE:
            lastPicked = NULL;
			if(!getSimulationManager()->isSimulationFresh())
            {
				StopSimulation();
                getSimulationManager()->RestartScenario();
            }
            StartSimulation();
            break;
            
        case SDLK_h:
            displayHUD = !displayHUD;
            break;
            
        case SDLK_c:
            displayConsole = !displayConsole;
            ((OpenGLConsole*)console)->ResetScroll();
            break;
			
		case SDLK_w: //Forward
		{
			OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
            if(trackball->isEnabled())
                trackball->MoveCenter(trackball->GetLookingDirection() * 0.1);
		}
			break;
			
		case SDLK_s: //Backward
		{
			OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
            if(trackball->isEnabled())
                trackball->MoveCenter(-trackball->GetLookingDirection() * 0.1);
		}
			break;
			
		case SDLK_a: //Left
		{
			OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
            if(trackball->isEnabled())
            {
                glm::vec3 axis = glm::cross(trackball->GetLookingDirection(), trackball->GetUpDirection());
                trackball->MoveCenter(-axis * 0.1);
            }
		}
			break;
			
		case SDLK_d: //Right
		{
			OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
            if(trackball->isEnabled())
            {
                glm::vec3 axis = glm::cross(trackball->GetLookingDirection(), trackball->GetUpDirection());
                trackball->MoveCenter(axis * 0.1);
            }
		}
			break;
            
        case SDLK_q: //Up
        {
            OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
            if(trackball->isEnabled())
                trackball->MoveCenter(glm::vec3(0,0,0.1));
        }
            break;
            
        case SDLK_z: //Down
        {
            OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
            if(trackball->isEnabled())
                trackball->MoveCenter(glm::vec3(0,0,-0.1));
        }
            break;

        default:
            break;
    }
}

void GraphicalSimulationApp::KeyUp(SDL_Event *event)
{
}

void GraphicalSimulationApp::MouseDown(SDL_Event *event)
{
}

void GraphicalSimulationApp::MouseUp(SDL_Event *event)
{
}

void GraphicalSimulationApp::MouseMove(SDL_Event *event)
{
}

void GraphicalSimulationApp::MouseScroll(SDL_Event *event)
{
}

void GraphicalSimulationApp::JoystickDown(SDL_Event *event)
{
}

void GraphicalSimulationApp::JoystickUp(SDL_Event *event)
{
}

void GraphicalSimulationApp::ProcessInputs()
{
}

void GraphicalSimulationApp::Loop()
{
    SDL_Event event;
    bool mouseWasDown = false;
    
    uint64_t startTime = GetTimeInMicroseconds();
    
    while(!hasFinished())
    {
        SDL_FlushEvents(SDL_FINGERDOWN, SDL_MULTIGESTURE);
            
        while(SDL_PollEvent(&event))
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
                    gui->KeyDown(event.key.keysym.sym);
                    KeyDown(&event);
                    break;
                }
                    
                case SDL_KEYUP:
                {
                    gui->KeyUp(event.key.keysym.sym);
                    KeyUp(&event);
                    break;
                }
                    
                case SDL_MOUSEBUTTONDOWN:
                {
                    gui->MouseDown(event.button.x, event.button.y, event.button.button == SDL_BUTTON_LEFT);
                    mouseWasDown = true;
                }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                {
                    //GUI
                    gui->MouseUp(event.button.x, event.button.y, event.button.button == SDL_BUTTON_LEFT);
                    
                    //Trackball
                    if(event.button.button == SDL_BUTTON_RIGHT || event.button.button == SDL_BUTTON_MIDDLE)
                    {
                        OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
                        if(trackball->isEnabled())
                            trackball->MouseUp();
                    }
                    
                    //Pass
                    MouseUp(&event);
                }
                    break;
                    
                case SDL_MOUSEMOTION:
                {
                    //GUI
                    gui->MouseMove(event.motion.x, event.motion.y);
                    
                    OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
                    if(trackball->isEnabled())
                    {
                        GLfloat xPos = (GLfloat)(event.motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
                        GLfloat yPos = -(GLfloat)(event.motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
                        trackball->MouseMove(xPos, yPos);
                    }
                        
                    //Pass
                    MouseMove(&event);
                }
                    break;
                    
                case SDL_MOUSEWHEEL:
                {
                    if(displayConsole) //GUI
                        ((OpenGLConsole*)console)->Scroll((GLfloat)-event.wheel.y);
                    else
                    {
                        //Trackball
                        OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
                        if(trackball->isEnabled())
                            trackball->MouseScroll(event.wheel.y * -1.f);
                        
                        //Pass
                        MouseScroll(&event);
                    }
                }
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
                {
                    if(isRunning())
                        StopSimulation();
                        
                    Quit();
                }   
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
        RenderLoop();
		
        //workaround for checking if IMGUI is being manipulated
        if(mouseWasDown && !gui->isAnyActive())
        {
            OpenGLTrackball* trackball = getSimulationManager()->getTrackball();
            
            if(trackball->isEnabled())
            {
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    glm::vec3 eye = trackball->GetEyePosition();
                    glm::vec3 ray = trackball->Ray(event.button.x, event.button.y);
                    lastPicked = getSimulationManager()->PickEntity(Vector3(-eye.x, eye.y, -eye.z), Vector3(-ray.x, ray.y, -ray.z));
                }
                else //RIGHT OR MIDDLE
                {
                    GLfloat xPos = (GLfloat)(event.motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
                    GLfloat yPos = -(GLfloat)(event.motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
                    trackball->MouseDown(xPos, yPos, event.button.button == SDL_BUTTON_MIDDLE);
                }
            }
            
            //Pass
            MouseDown(&event);
        }
        mouseWasDown = false;

        //Framerate limitting
        uint64_t elapsedTime = GetTimeInMicroseconds() - startTime;
        if(elapsedTime < 16000)
            std::this_thread::sleep_for(std::chrono::microseconds(16000 - elapsedTime));
        startTime = GetTimeInMicroseconds();
    }
}

void GraphicalSimulationApp::RenderLoop()
{
    //Do some updates
    if(!isRunning())
    {
        getSimulationManager()->UpdateDrawingQueue();
    }
	
    //Rendering
    uint64_t startTime = GetTimeInMicroseconds();
	glPipeline->Render(getSimulationManager());
    glPipeline->DrawDisplay();
    
    //GUI & Console
    if(displayConsole)
    {
        gui->GenerateBackground();
        ((OpenGLConsole*)console)->Render(true);
    }
    else
    {
        if(displayHUD) //Draw immediate mode GUI
        {
            gui->GenerateBackground();
            gui->Begin();
            DoHUD();
            gui->End();
        }
        else //Just draw logo in the corner
        {
            gui->Begin();
            gui->End();
        }
    }
	
	SDL_GL_SwapWindow(window);
	drawingTime = (GetTimeInMicroseconds() - startTime)/1000.0; //in ms
}

void GraphicalSimulationApp::DoHUD()
{
    char buf[256];
    
    //Helper settings
    HelperSettings& hs = getHelperSettings();
    Ocean* ocn = getSimulationManager()->getOcean();
    
    GLfloat offset = 10.f;
    gui->DoPanel(10.f, offset, 160.f, ocn != NULL ? 182.f : 137.f);
    offset += 5.f;
    gui->DoLabel(15.f, offset, "HELPERS");
    offset += 15.f;
    
    Uid id;
    id.owner = 0;
    id.index = 0;
    id.item = 1;
    hs.showCoordSys = gui->DoCheckBox(id, 15.f, offset, 110.f, hs.showCoordSys, "Frames");
    offset += 22.f;
    
    id.item = 2;
    hs.showSensors = gui->DoCheckBox(id, 15.f, offset, 110.f, hs.showSensors, "Sensors");
    offset += 22.f;
    
    id.item = 3;
    hs.showActuators = gui->DoCheckBox(id, 15.f, offset, 110.f, hs.showActuators, "Actuators");
    offset += 22.f;
    
    id.item = 4;
    hs.showJoints = gui->DoCheckBox(id, 15.f, offset, 110.f, hs.showJoints, "Joints");
    offset += 22.f;
    
    id.item = 5;
    hs.showBulletDebugInfo = gui->DoCheckBox(id, 15.f, offset, 110.f, hs.showBulletDebugInfo, "Collision");
    offset += 22.f;
    
    if(ocn != NULL)
    {
        id.item = 6;
        hs.showForces = gui->DoCheckBox(id, 15.f, offset, 110.f, hs.showForces, "Fluid Forces");
        offset += 22.f;
    
        id.item = 7;
        hs.showFluidDynamics = gui->DoCheckBox(id, 15.f, offset, 110.f, hs.showFluidDynamics, "Hydrodynamics");
        offset += 22.f;
    }
    
    offset += 14.f;
    
    //Time settings
    Scalar az, elev;
    getSimulationManager()->getAtmosphere()->GetSunPosition(az, elev);
    
    gui->DoPanel(10.f, offset, 160.f, 125.f);
    offset += 5.f;
    gui->DoLabel(15.f, offset, "SUN POSITION");
    offset += 15.f;
    
    id.owner = 1;
    id.index = 0;
    id.item = 0;
    az = gui->DoSlider(id, 15.f, offset, 150.f, Scalar(-180), Scalar(180), az, "Azimuth[deg]");
    offset += 50.f;
    
    id.item = 1;
    elev = gui->DoSlider(id, 15.f, offset, 150.f, Scalar(-10), Scalar(90), elev, "Elevation[deg]");
    offset += 61.f;
    
    getSimulationManager()->getAtmosphere()->SetupSunPosition(az, elev);
    
    //Ocean settings
    if(ocn != NULL)
    {
        Scalar waterType = ocn->getWaterType();
        Scalar turbidity = ocn->getTurbidity();
        
        bool oceanOn = ocn->isRenderable();
        
        gui->DoPanel(10.f, offset, 160.f, oceanOn ? 132.f : 33.f);
        offset += 5.f;
       
        id.owner = 2;
        id.index = 0;
        id.item = 0;
        ocn->setRenderable(gui->DoCheckBox(id, 15.f, offset, 110.f, oceanOn, "OCEAN"));
        offset += 26.f;
        
        if(oceanOn)
        {
            id.item = 1;
            waterType = gui->DoSlider(id, 15.f, offset, 150.f, Scalar(0), Scalar(1), waterType, "Water Type");
            offset += 50.f;
        
            id.item = 2;
            turbidity = gui->DoSlider(id, 15.f, offset, 150.f, Scalar(0.1), Scalar(100.0), turbidity, "Turbidity");
            offset += 50.f;
        
            ocn->SetupWaterProperties(waterType, turbidity);
        }
        
        offset += 7.f;
    }
    
    //Main view exposure
    gui->DoPanel(10.f, offset, 160.f, 75.f);
    offset += 5.f;
    gui->DoLabel(15.f, offset, "RENDERING");
    offset += 15.f;
    
    id.owner = 3;
    id.index = 0;
    id.item = 0;
    getSimulationManager()->getTrackball()->setExposureCompensation(gui->DoSlider(id, 15.f, offset, 150.f, Scalar(-3), Scalar(3), getSimulationManager()->getTrackball()->getExposureCompensation(), "Exposure[EV]"));
    offset += 61.f;
    
    //Picked entity information
    if(lastPicked != NULL)
    {
        switch(lastPicked->getType())
        {
            case EntityType::ENTITY_STATIC:
            {
                StaticEntity* ent = (StaticEntity*)lastPicked;
                
                gui->DoPanel(10.f, offset, 160.f, 66.f);
                offset += 5.f;
                gui->DoLabel(15.f, offset, "SELECTION INFO");
                offset += 16.f;
                gui->DoLabel(18.f, offset, std::string("Name: ") + ent->getName());
                offset += 14.f;
                gui->DoLabel(18.f, offset, std::string("Type: Static"));
                offset += 14.f;
                gui->DoLabel(18.f, offset, std::string("Material: ") + ent->getMaterial().name);
            }
                break;
                
            case EntityType::ENTITY_SOLID:
            {
                SolidEntity* ent = (SolidEntity*)lastPicked;
                
                gui->DoPanel(10.f, offset, 160.f, 109.f);
                offset += 5.f;
                gui->DoLabel(15.f, offset, "SELECTION INFO");
                offset += 16.f;
                gui->DoLabel(18.f, offset, std::string("Name: ") + ent->getName());
                offset += 14.f;
                gui->DoLabel(18.f, offset, std::string("Type: Dynamic"));
                offset += 14.f;
                gui->DoLabel(18.f, offset, ent->getSolidType() == SolidType::SOLID_COMPOUND ? std::string("Material: Compound") : std::string("Material: ") + ent->getMaterial().name);
                offset += 14.f;
                std::sprintf(buf, "%1.3lf", ent->getMass());
                gui->DoLabel(18.f, offset, std::string("Mass[kg]: ") + std::string(buf));
                offset += 14.f;
                gui->DoLabel(18.f, offset, std::string("Inertia[kgm2]: "));
                offset += 14.f;
                Vector3 I = ent->getInertia();
                std::sprintf(buf, "%1.3lf, %1.3lf, %1.3lf", I.x(), I.y(), I.z());
                gui->DoLabel(23.f, offset, std::string(buf));
            }
                break;
                
            default:
                break;
        }
    }
    
    //Bottom panel
    gui->DoPanel(0, getWindowHeight()-30.f, getWindowWidth(), 30.f);
    
	std::sprintf(buf, "Drawing time: %1.2lf ms", getDrawingTime());
    gui->DoLabel(10, getWindowHeight() - 20.f, buf);
    
    std::sprintf(buf, "CPU usage: %1.0lf%%", getSimulationManager()->getCpuUsage());
    gui->DoLabel(170, getWindowHeight() - 20.f, buf);
    
    std::sprintf(buf, "Simulation time: %1.2lf s", getSimulationManager()->getSimulationTime());
    gui->DoLabel(290, getWindowHeight() - 20.f, buf);
}

void GraphicalSimulationApp::StartSimulation()
{
	SimulationApp::StartSimulation();
	
    GraphicalSimulationThreadData* data = new GraphicalSimulationThreadData();
    data->app = this;
    data->drawMutex = glPipeline->getDrawingQueueMutex();
    simulationThread = SDL_CreateThread(GraphicalSimulationApp::RunSimulation, "simulationThread", data);
}

void GraphicalSimulationApp::ResumeSimulation()
{
    SimulationApp::ResumeSimulation();
	
    GraphicalSimulationThreadData* data = new GraphicalSimulationThreadData();
    data->app = this;
    data->drawMutex = glPipeline->getDrawingQueueMutex();
    simulationThread = SDL_CreateThread(GraphicalSimulationApp::RunSimulation, "simulationThread", data);
}

void GraphicalSimulationApp::StopSimulation()
{
	SimulationApp::StopSimulation();
    
    int status;
    SDL_WaitThread(simulationThread, &status);
    simulationThread = NULL;
}

void GraphicalSimulationApp::CleanUp()
{
    SimulationApp::CleanUp();
    
    if(joystick != NULL)
        SDL_JoystickClose(0);
    
    if(glLoadingContext != NULL)
        SDL_GL_DeleteContext(glLoadingContext);
    
    SDL_GL_DeleteContext(glMainContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int GraphicalSimulationApp::RenderLoadingScreen(void* data)
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
        ((OpenGLConsole*)ltdata->app->getConsole())->Render(false);
        SDL_UnlockMutex(ltdata->mutex);
        
        SDL_GL_SwapWindow(ltdata->app->window);
    }
	
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
    
    //Detach thread from GL context
    SDL_GL_MakeCurrent(ltdata->app->window, NULL);
    return 0;
}

int GraphicalSimulationApp::RunSimulation(void* data)
{
    GraphicalSimulationThreadData* stdata = (GraphicalSimulationThreadData*)data;
    SimulationManager* sim = stdata->app->getSimulationManager();
    
    while(stdata->app->isRunning())
    {
        sim->AdvanceSimulation();
        if(stdata->app->getGLPipeline()->isDrawingQueueEmpty())
        {
            SDL_LockMutex(stdata->drawMutex);
            sim->UpdateDrawingQueue();
            SDL_UnlockMutex(stdata->drawMutex);
        }
	}
    
    return 0;
}

}
