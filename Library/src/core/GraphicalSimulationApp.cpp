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
//  Copyright (c) 2012-2026 Patryk Cieslak. All rights reserved.
//

#include "core/GraphicalSimulationApp.h"

#include <chrono>
#include <thread>
#include "core/SimulationManager.h"
#include "core/Robot.h"
#include "graphics/OpenGLState.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLConsole.h"
#include "graphics/OpenGLPrinter.h"
#include "graphics/IMGUI.h"
#include "graphics/OpenGLTrackball.h"
#include "utils/SystemUtil.hpp"
#include "entities/Entity.h"
#include "entities/StaticEntity.h"
#include "entities/SolidEntity.h"
#include "entities/MovingEntity.h"
#include "entities/solids/Compound.h"
#include "utils/icon.h"

namespace sf
{

GraphicalSimulationApp::GraphicalSimulationApp(const std::string& title, const std::string& dataDirPath, 
    RenderSettings r, HelperSettings h, std::unique_ptr<SimulationManager> sim)
: SimulationApp(title, dataDirPath, std::move(sim))
{
#ifdef SHADER_DIR_PATH
    shaderPath_ = SHADER_DIR_PATH;
#else
    shaderPath = "/usr/local/share/Stonefish/shaders/";
#endif
    glLoadingContext_ = nullptr;
    glMainContext_ = nullptr;
    trackball_ = nullptr;
    trackballCenter_ = nullptr;
    selectedEntity_ = std::make_pair(nullptr, -1);
    displayHUD_ = true;
    displayKeymap_ = false;
    displayConsole_ = false;
    displayPerformance_ = false;
    joystick_ = nullptr;
    mouseWasDown_.type = SDL_LASTEVENT;
    simulationThread_ = nullptr;
    loadingThread_ = nullptr;
    glPipeline_ = nullptr;
    gui_ = nullptr;
    timeQuery_[0] = 0;
    timeQuery_[1] = 0;
    timeQueryPingpong_ = 0;
    drawingTime_ = 0.0;
    fps_ = 0.0;
    maxDrawingTime_ = 0.0;
    maxCounter_ = 0;
    rSettings_ = r;
    hSettings_ = h;
    rSettings_.windowW += rSettings_.windowW % 2;
    rSettings_.windowH += rSettings_.windowH % 2;
    windowW_ = rSettings_.windowW;
    windowH_ = rSettings_.windowH;
}

void GraphicalSimulationApp::ShowHUD()
{
    displayHUD_ = true;
}

void GraphicalSimulationApp::HideHUD()
{
    displayHUD_ = false;
}

void GraphicalSimulationApp::ShowConsole()
{
    displayConsole_ = true;
}

void GraphicalSimulationApp::HideConsole()
{
    displayConsole_ = false;
}

OpenGLPipeline* GraphicalSimulationApp::getGLPipeline()
{
    return glPipeline_.get();
}

IMGUI* GraphicalSimulationApp::getGUI()
{
    return gui_.get();
}

OpenGLTrackball* GraphicalSimulationApp::getTrackball()
{
    return trackball_;
}

std::pair<Entity*, int> GraphicalSimulationApp::getSelectedEntity()
{
    return selectedEntity_;
}

bool GraphicalSimulationApp::hasGraphics()
{
    return true;
}

SDL_Joystick* GraphicalSimulationApp::getJoystick()
{
    return joystick_;
}

double GraphicalSimulationApp::getDrawingTime(bool max)
{
    if(max)
        return maxDrawingTime_;
    else
        return drawingTime_;
}

int GraphicalSimulationApp::getWindowWidth()
{
    return windowW_;
}

int GraphicalSimulationApp::getWindowHeight()
{
    return windowH_;
}

std::string GraphicalSimulationApp::getShaderPath()
{
    return shaderPath_;
}

RenderSettings GraphicalSimulationApp::getRenderSettings() const
{
    return glPipeline_->getRenderSettings();
}

HelperSettings& GraphicalSimulationApp::getHelperSettings()
{
    return glPipeline_->getHelperSettings();
}

void GraphicalSimulationApp::Init()
{
    //General initialization
    SimulationApp::Init();
    //Window initialization + loading thread
    loading_ = true;
    InitializeSDL();

    //Continue initialization with console visible
    cInfo("Initializing rendering pipeline:");
    cInfo("Loading GUI...");
    gui_ = std::make_unique<IMGUI>(windowW_, windowH_);
    InitializeGUI(); //Initialize non-standard graphical elements
    glPipeline_ = std::make_unique<OpenGLPipeline>(rSettings_, hSettings_);
    ShowHUD();
    
    cInfo("Initializing simulation:");
    InitializeSimulation();
    
    cInfo("Ready for running...");
    SDL_Delay(1000);
    
    //Close loading console - exit loading thread
    loading_ = false;
    int status = 0;
    SDL_WaitThread(loadingThread_, &status);
    SDL_GL_MakeCurrent(window_, glMainContext_);

    //Create performance counters
    glGenQueries(2, timeQuery_);

    state_ = SimulationState::STOPPED;
}

void GraphicalSimulationApp::InitializeSDL()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK);
    
    //Create OpenGL contexts
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    
    //Create window
    window_ = SDL_CreateWindow(getName().c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              windowW_,
                              windowH_,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN// | SDL_WINDOW_ALLOW_HIGHDPI
                              );
                              
    //Set window icon
    uint32_t rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = (icon_image.bytes_per_pixel == 3) ? 8 : 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = (icon_image.bytes_per_pixel == (3)) ? 0 : 0xff000000;
#endif
                              
    SDL_Surface* icon = SDL_CreateRGBSurfaceFrom((void*)icon_image.pixel_data, icon_image.width, icon_image.height, 
        icon_image.bytes_per_pixel*8, icon_image.bytes_per_pixel*icon_image.width, rmask, gmask, bmask, amask);
    
    SDL_SetWindowIcon(window_, icon);
    SDL_FreeSurface(icon);
    
    //Create OpenGL contexts
    glLoadingContext_ = SDL_GL_CreateContext(window_);
    if(glLoadingContext_ == nullptr)
        cCritical("SDL2: %s", SDL_GetError());
    
    glMainContext_ = SDL_GL_CreateContext(window_);
    if(glMainContext_ == nullptr)
        cCritical("SDL2: %s", SDL_GetError());
    
    //Disable vertical synchronization
    if(!rSettings_.verticalSync)
    {
        if(SDL_GL_SetSwapInterval(0) == -1)
            cError("SDL2: %s", SDL_GetError());
    }
    
    //Initialize OpenGL function handlers 
    int version = gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    int vmajor = GLAD_VERSION_MAJOR(version);
    int vminor = GLAD_VERSION_MINOR(version);
    if(vmajor < 4 || (vmajor == 4 && vminor < 3))
        cCritical("This program requires support for OpenGL 4.3, however OpenGL %d.%d was detected! Exiting...", vmajor, vminor);

    //Initialize OpenGL pipeline
    cInfo("Window created. OpenGL %d.%d contexts created.", vmajor, vminor);
    OpenGLState::Init();
    GLSLShader::Init();
    OpenGLPrinter::Init();
    
    //Initialize console output
    std::vector<ConsoleMessage> textLines = console_->getLines();
    console_.reset();
    console_ = std::make_unique<OpenGLConsole>();
    for(size_t i=0; i<textLines.size(); ++i)
        console_->AppendMessage(textLines[i]);
    static_cast<OpenGLConsole*>(console_.get())->Init(windowW_, windowH_);
    
    //Create loading thread
    GraphicalSimulationThreadData* data = new GraphicalSimulationThreadData{*this};
    loadingThread_ = SDL_CreateThread(GraphicalSimulationApp::RenderLoadingScreen, "loadingThread", data);
    
    //Look for joysticks
    int jcount = SDL_NumJoysticks();
    
    if(jcount > 0)
    {
        joystick_ = SDL_JoystickOpen(0);
        joystickButtons_.resize(SDL_JoystickNumButtons(joystick_), false);
        joystickAxes_.resize(SDL_JoystickNumAxes(joystick_), 0);
        joystickHats_.resize(SDL_JoystickNumHats(joystick_), 0);
        cInfo("Joystick %s connected (%d axes, %d hats, %d buttons)", SDL_JoystickName(joystick_),
                                                                      SDL_JoystickNumAxes(joystick_),
                                                                      SDL_JoystickNumHats(joystick_),
                                                                      SDL_JoystickNumButtons(joystick_));
    }
}

void GraphicalSimulationApp::InitializeGUI()
{
}

void GraphicalSimulationApp::CreateTrackball()
{
    if(getGLPipeline()->getContent()->getViewsCount() == 0)
    {
        std::unique_ptr<OpenGLTrackball> trackball = std::make_unique<OpenGLTrackball>(glm::vec3(0.f,0.f,-1.f), 5.0, glm::vec3(0.f,0.f,-1.f), 0, 0, 
            getWindowWidth(), getWindowHeight(), 90.f, glm::vec2(STD_NEAR_PLANE_DISTANCE, STD_FAR_PLANE_DISTANCE));
        trackball_ = trackball.get();
        trackball_->Rotate(glm::quat(glm::eulerAngleYXZ(0.0, 0.0, 0.25)));
        getGLPipeline()->getContent()->AddView(std::move(trackball));
    }
}

void GraphicalSimulationApp::WindowEvent(SDL_Event* event)
{
    int w, h;
    
    switch(event->window.event)
    {
        case SDL_WINDOWEVENT_RESIZED:
            SDL_GetWindowSize(window_, &w, &h);
            gui_->Resize(w, h);
            break;
    }
}

void GraphicalSimulationApp::KeyDown(SDL_Event *event)
{
    GLfloat moveStep = 0.1f;
    if(event->key.keysym.mod & KMOD_SHIFT)
        moveStep = 1.f;

    switch (event->key.keysym.sym)
    {
        case SDLK_ESCAPE:
            Quit();
            break;
                      
        case SDLK_h:
            displayHUD_ = !displayHUD_;
            break;

        case SDLK_k:
            displayKeymap_ = !displayKeymap_;
            break;

        case SDLK_p:
            displayPerformance_ = !displayPerformance_;
            break;

        case SDLK_c:
            displayConsole_ = !displayConsole_;
            static_cast<OpenGLConsole*>(console_.get())->ResetScroll();
            break;
            
        case SDLK_w: //Forward
        {
            if(trackball_->isEnabled())
                trackball_->MoveCenter(trackball_->GetLookingDirection() * moveStep);
        }
            break;
            
        case SDLK_s: //Backward
        {
            if(trackball_->isEnabled())
                trackball_->MoveCenter(-trackball_->GetLookingDirection() * moveStep);
        }
            break;
            
        case SDLK_a: //Left
        {
            if(trackball_->isEnabled())
            {
                glm::vec3 axis = glm::cross(trackball_->GetLookingDirection(), trackball_->GetUpDirection());
                trackball_->MoveCenter(-axis * moveStep);
            }
        }
            break;
            
        case SDLK_d: //Right
        {
            if(trackball_->isEnabled())
            {
                glm::vec3 axis = glm::cross(trackball_->GetLookingDirection(), trackball_->GetUpDirection());
                trackball_->MoveCenter(axis * moveStep);
            }
        }
            break;
            
        case SDLK_q: //Up
        {
            if(trackball_->isEnabled())
                trackball_->MoveCenter(glm::vec3(0.f, 0.f, -moveStep));
        }
            break;
            
        case SDLK_z: //Down
        {
            if(trackball_->isEnabled())
                trackball_->MoveCenter(glm::vec3(0.f, 0.f, moveStep));
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

void GraphicalSimulationApp::LoopInternal()
{
    SDL_Event event;
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
                gui_->KeyDown(event.key.keysym.sym);
                KeyDown(&event);
                break;
            }
                
            case SDL_KEYUP:
            {
                gui_->KeyUp(event.key.keysym.sym);
                KeyUp(&event);
                break;
            }
                
            case SDL_MOUSEBUTTONDOWN:
            {
                gui_->MouseDown(event.button.x, event.button.y, event.button.button == SDL_BUTTON_LEFT);
                mouseWasDown_ = event;
            }
                break;
                
            case SDL_MOUSEBUTTONUP:
            {
                //GUI
                gui_->MouseUp(event.button.x, event.button.y, event.button.button == SDL_BUTTON_LEFT);
                
                //Trackball
                if(event.button.button == SDL_BUTTON_RIGHT || event.button.button == SDL_BUTTON_MIDDLE)
                {
                    if(trackball_->isEnabled())
                        trackball_->MouseUp();
                }
                
                //Pass
                MouseUp(&event);
            }
                break;
                
            case SDL_MOUSEMOTION:
            {
                //GUI
                gui_->MouseMove(event.motion.x, event.motion.y);
                
                if(trackball_->isEnabled())
                {
                    GLfloat xPos = (GLfloat)(event.motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
                    GLfloat yPos = -(GLfloat)(event.motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
                    trackball_->MouseMove(xPos, yPos);
                }
                    
                //Pass
                MouseMove(&event);
            }
                break;
                
            case SDL_MOUSEWHEEL:
            {
                if(displayConsole_) //GUI
                    static_cast<OpenGLConsole*>(console_.get())->Scroll((GLfloat)-event.wheel.y);
                else
                {
                    //Trackball
                    if(trackball_->isEnabled())
                        trackball_->MouseScroll(event.wheel.y * -1.f);
                    
                    //Pass
                    MouseScroll(&event);
                }
            }
                break;
                
            case SDL_JOYBUTTONDOWN:
                joystickButtons_[event.jbutton.button] = true;
                JoystickDown(&event);
                break;
                
            case SDL_JOYBUTTONUP:
                joystickButtons_[event.jbutton.button] = false;
                JoystickUp(&event);
                break;
                
            case SDL_QUIT:
            {
                if(state_ == SimulationState::RUNNING)
                    StopSimulation();
                    
                Quit();
            }   
                break;
        }
    }

    if(joystick_ != nullptr)
    {
        for(int i=0; i<SDL_JoystickNumAxes(joystick_); i++)
            joystickAxes_[i] = SDL_JoystickGetAxis(joystick_, i);
    
        for(int i=0; i<SDL_JoystickNumHats(joystick_); i++)
            joystickHats_[i] = SDL_JoystickGetHat(joystick_, i);
    }
    
    ProcessInputs();
    RenderLoop();
    
    //workaround for checking if IMGUI is being manipulated
    if(mouseWasDown_.type == SDL_MOUSEBUTTONDOWN && !gui_->isAnyActive())
    {
        if(trackball_->isEnabled())
        {
            if(mouseWasDown_.button.button == SDL_BUTTON_LEFT)
            {
                glm::vec3 eye = trackball_->GetEyePosition();
                glm::vec3 ray = trackball_->Ray(mouseWasDown_.button.x, mouseWasDown_.button.y);
                selectedEntity_ = getSimulationManager()->PickEntity(Vector3(eye.x, eye.y, eye.z), Vector3(ray.x, ray.y, ray.z));
            }
            else //RIGHT OR MIDDLE
            {
                GLfloat xPos = (GLfloat)(mouseWasDown_.motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
                GLfloat yPos = -(GLfloat)(mouseWasDown_.motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
                trackball_->MouseDown(xPos, yPos, mouseWasDown_.button.button == SDL_BUTTON_MIDDLE);
            }
        }
        
        //Pass
        MouseDown(&event);
    }
    mouseWasDown_.type = SDL_LASTEVENT;

    // Update FPS
    uint64_t elapsedTime = GetTimeInMicroseconds() - startTime_;
    startTime_ = GetTimeInMicroseconds();
    double dt = std::min(elapsedTime/1000000.0 + 1e-6, 1.0); //in s, secure against large values at the beginning
	constexpr double f = 1.0/60.0;
	fps_ = f*(1.0/dt) + (1.0-f)*fps_;
}

void GraphicalSimulationApp::RenderLoop()
{
    //Do some updates
    if(state_ != SimulationState::RUNNING)
    {
        getSimulationManager()->UpdateDrawingQueue();
    }
    
    //Rendering
    glBeginQuery(GL_TIME_ELAPSED, timeQuery_[timeQueryPingpong_]);
    glPipeline_->Render(getSimulationManager());
    glPipeline_->DrawDisplay();
    
    //GUI & Console
    if(displayConsole_)
    {
        gui_->GenerateBackground();
        static_cast<OpenGLConsole*>(console_.get())->Render(true);
    }
    else
    {
        if(displayHUD_) //Draw immediate mode GUI
        {
            gui_->GenerateBackground();
            gui_->Begin();
            DoHUD();
            gui_->End();
        }
        else //Just draw logo in the corner
        {
            gui_->Begin();
            gui_->End();
        }
    }
    glEndQuery(GL_TIME_ELAPSED);
    
    //Update drawing time
    uint64_t drawTime;
    glGetQueryObjectui64v(timeQuery_[1-timeQueryPingpong_], GL_QUERY_RESULT, &drawTime);
    timeQueryPingpong_ = 1-timeQueryPingpong_;
	double dt = std::min(drawTime/1000000.0, 1000.0); //in ms
	double f = 1.0/60.0;
	drawingTime_ = f*dt + (1.0-f)*drawingTime_;

    //Update maximum drawing time
    if(maxCounter_ >= 60)
    {
        maxDrawingTime_ = drawingTime_;
        maxCounter_ = 0;
    }
    else
    {
        maxDrawingTime_ = std::max(maxDrawingTime_, dt);
        ++maxCounter_;
    }

    //glFinish(); //Ensure that the frame was fully rendered
    SDL_GL_SwapWindow(window_);
}

void GraphicalSimulationApp::DoHUD()
{
    char buf[256];
    
    //Helper settings
    HelperSettings& hs = getHelperSettings();
    Ocean* ocn = getSimulationManager()->getOcean();
    
    GLfloat offset = 10.f;
    gui_->DoPanel(10.f, offset, 160.f, ocn != nullptr ? 226.f : 159.f);
    offset += 5.f;
    gui_->DoLabel(15.f, offset, "DEBUG");
    offset += 15.f;
    
    Uid id;
    id.owner = 0;
    
    id.item = 0;
    bool displayPhysical = getSimulationManager()->getSolidDisplayMode() == DisplayMode::PHYSICAL; 
    displayPhysical = gui_->DoCheckBox(id, 15.f, offset, 110.f, displayPhysical, "Physical objects");
    getSimulationManager()->setSolidDisplayMode(displayPhysical ? DisplayMode::PHYSICAL : DisplayMode::GRAPHICAL);
    offset += 22.f;
    
    id.item = 1;
    hs.showCoordSys = gui_->DoCheckBox(id, 15.f, offset, 110.f, hs.showCoordSys, "Frames");
    offset += 22.f;
    
    id.item = 2;
    hs.showSensors = gui_->DoCheckBox(id, 15.f, offset, 110.f, hs.showSensors, "Sensors");
    offset += 22.f;
    
    id.item = 3;
    hs.showActuators = gui_->DoCheckBox(id, 15.f, offset, 110.f, hs.showActuators, "Actuators");
    offset += 22.f;
    
    id.item = 4;
    hs.showJoints = gui_->DoCheckBox(id, 15.f, offset, 110.f, hs.showJoints, "Joints");
    offset += 22.f;
    
    id.item = 5;
    hs.showBulletDebugInfo = gui_->DoCheckBox(id, 15.f, offset, 110.f, hs.showBulletDebugInfo, "Collision");
    offset += 22.f;
    
    if(ocn != nullptr)
    {
        id.item = 6;
        hs.showForces = gui_->DoCheckBox(id, 15.f, offset, 110.f, hs.showForces, "Fluid Forces");
        offset += 22.f;
    
        id.item = 7;
        hs.showFluidDynamics = gui_->DoCheckBox(id, 15.f, offset, 110.f, hs.showFluidDynamics, "Hydrodynamics");
        offset += 22.f;

        id.item = 8;
        hs.showOceanVelocityField = gui_->DoCheckBox(id, 15.f, offset, 110.f, hs.showOceanVelocityField, "Water velocity");
        offset += 22.f;
    }
    
    offset += 14.f;
    
    //Time settings
    Scalar az, elev;
    getSimulationManager()->getAtmosphere()->GetSunPosition(az, elev);
    
    gui_->DoPanel(10.f, offset, 160.f, 125.f);
    offset += 5.f;
    gui_->DoLabel(15.f, offset, "SUN POSITION");
    offset += 15.f;
    
    id.owner = 1;
    id.item = 0;
    az = gui_->DoSlider(id, 15.f, offset, 150.f, Scalar(-180), Scalar(180), az, "Azimuth[deg]");
    offset += 50.f;
    
    id.item = 1;
    elev = gui_->DoSlider(id, 15.f, offset, 150.f, Scalar(-10), Scalar(90), elev, "Elevation[deg]");
    offset += 61.f;
    
    getSimulationManager()->getAtmosphere()->SetSunPosition(az, elev);
    
    //Ocean settings
    if(ocn != nullptr)
    {
        Scalar waterType = ocn->getWaterType();
        
        bool oceanOn = ocn->isRenderable();
        
        gui_->DoPanel(10.f, offset, 160.f, oceanOn ? 112.f : 33.f);
        offset += 5.f;
       
        id.owner = 2;
        id.item = 0;
        ocn->setRenderable(gui_->DoCheckBox(id, 15.f, offset, 110.f, oceanOn, "OCEAN"));
        offset += 26.f;
        
        if(oceanOn)
        {
            id.item = 1;
            waterType = gui_->DoSlider(id, 15.f, offset, 150.f, Scalar(0), Scalar(1), waterType, "Jerlov water type");
            ocn->setWaterType(waterType);
            offset += 50.f;
            
            id.item = 2;
            ocn->setParticles(gui_->DoCheckBox(id, 19.f, offset, 110.f, ocn->hasParticles(), "Suspended particles"));
            offset += 29.f;
        }

        offset += 8.f;
    }
    
    //Main view exposure
    gui_->DoPanel(10.f, offset, 160.f, 126.f);
    offset += 5.f;
    gui_->DoLabel(15.f, offset, "VIEW");
    offset += 15.f;
    
    id.owner = 3;
    id.item = 0;
    std::vector<std::string> options;
    options.push_back("Free");

    unsigned int selected = 0;
    unsigned int newSelected = 0;

    //Add robots to the list
    size_t rid = 0;
    Robot* rob;
    while((rob = getSimulationManager()->getRobot(rid)) != nullptr)
    {
        options.push_back(rob->getName());
        if(rob->getBaseLink() == trackballCenter_)
            selected = (unsigned int)(rid + 1);
        ++rid;
    }
    //Add animated entities to the list
    size_t eid = 0;
    size_t aid = 0;
    Entity* ent;
    while((ent = getSimulationManager()->getEntity(eid)) != nullptr)
    {
        if(ent->getType() == sf::EntityType::ANIMATED)
        {
            options.push_back(ent->getName());
            if(ent == trackballCenter_)
                selected = (unsigned int)(rid + 1 + aid);
            ++aid;
        }
        ++eid;
    }

    newSelected = gui_->DoComboBox(id, 15.f, offset, 150.f, options, selected, "Trackball center");
    
    if(newSelected != selected)
    {
        if(newSelected == 0)
            trackballCenter_ = nullptr;
        else
        {
            if(newSelected <= rid)
                trackballCenter_ = getSimulationManager()->getRobot(options[newSelected])->getBaseLink();
            else if(newSelected > rid)
                trackballCenter_ = (MovingEntity*)getSimulationManager()->getEntity(options[newSelected]);
        }     
        trackball_->GlueToMoving(trackballCenter_);
    }
    offset += 51.f;
    
    id.owner = 3;
    id.item = 1;
    trackball_->setExposureCompensation(gui_->DoSlider(id, 15.f, offset, 150.f, Scalar(-3), Scalar(3), trackball_->getExposureCompensation(), "Exposure[EV]"));
    offset += 61.f;
    
    //Picked entity information
    if(selectedEntity_.first != nullptr)
    {
        switch(selectedEntity_.first->getType())
        {
            case EntityType:: STATIC:
            {
                StaticEntity* ent = (StaticEntity*)selectedEntity_.first;
                
                gui_->DoPanel(10.f, offset, 160.f, 66.f);
                offset += 5.f;
                gui_->DoLabel(15.f, offset, "SELECTION INFO");
                offset += 16.f;
                gui_->DoLabel(18.f, offset, std::string("Name: ") + ent->getName());
                offset += 14.f;
                gui_->DoLabel(18.f, offset, std::string("Type: Static"));
                offset += 14.f;
                gui_->DoLabel(18.f, offset, std::string("Material: ") + ent->getMaterial().name);
            }
                break;
                
            case EntityType:: SOLID:
            {
                SolidEntity* ent = (SolidEntity*)selectedEntity_.first;
                
                GLfloat infoOffset = offset;
                gui_->DoPanel(10.f, offset, 160.f, ent->getSolidType() == SolidType::COMPOUND ? 130.f : 122.f);
                offset += 5.f;
                gui_->DoLabel(15.f, offset, "SELECTION INFO");
                offset += 16.f;
                gui_->DoLabel(18.f, offset, std::string("Name: ") + ent->getName());
                offset += 14.f;
                gui_->DoLabel(18.f, offset, std::string("Type: Dynamic"));
                offset += 14.f;
                if(ent->getSolidType() != SolidType::COMPOUND)
                {
                    gui_->DoLabel(18.f, offset, std::string("Material: ") + ent->getMaterial().name);
                    offset += 14.f;
                }
                std::sprintf(buf, "%1.3lf", ent->getMass());
                gui_->DoLabel(18.f, offset, std::string("Mass[kg]: ") + std::string(buf));
                offset += 14.f;
                gui_->DoLabel(18.f, offset, std::string("Inertia[kgm2]: "));
                offset += 14.f;
                Vector3 I = ent->getInertia();
                std::sprintf(buf, "%1.3lf, %1.3lf, %1.3lf", I.x(), I.y(), I.z());
                gui_->DoLabel(23.f, offset, std::string(buf));
                offset += 14.f;
                std::sprintf(buf, "%1.3lf", ent->getVolume()*1e3);
                gui_->DoLabel(18.f, offset, std::string("Volume[dm3]: ") + std::string(buf));
                offset += 11.f;
                
                if(ent->getSolidType() == SolidType::COMPOUND)
                {
                    Compound* cmp = (Compound*)ent;
                    id.owner = 4;
                    id.item = 0;
                    cmp->setDisplayInternalParts(gui_->DoCheckBox(id, 15.f, offset, 110.f, cmp->isDisplayingInternalParts(), "Show internals"));
                    offset += 22.f;

                    const CompoundPart& part = cmp->getPart(cmp->getPartId(selectedEntity_.second));
                    if(part.solid != nullptr)
                    {
                        offset = infoOffset + 10.f;
                        GLfloat hOffset = 165.f;
                        gui_->DoPanel(hOffset + 10.f, offset, 130.f, 110.f);
                        offset += 5.f;
                        gui_->DoLabel(hOffset + 15.f, offset, "PART INFO");
                        offset += 16.f;
                        std::string partName = part.solid->getName();
                        int beginIdx = partName.rfind('/');
                        gui_->DoLabel(hOffset + 18.f, offset, std::string("Name: ") + partName.substr(beginIdx + 1));
                        offset += 14.f;
                        gui_->DoLabel(hOffset + 18.f, offset, std::string("Material: ") + part.solid->getMaterial().name);
                        offset += 14.f;
                        std::sprintf(buf, "%1.3lf", part.solid->getMass());
                        gui_->DoLabel(hOffset + 18.f, offset, std::string("Mass[kg]: ") + std::string(buf));
                        offset += 14.f;
                        gui_->DoLabel(hOffset + 18.f, offset, std::string("Inertia[kgm2]: "));
                        offset += 14.f;
                        Vector3 I = part.solid->getInertia();
                        std::sprintf(buf, "%1.3lf, %1.3lf, %1.3lf", I.x(), I.y(), I.z());
                        gui_->DoLabel(hOffset + 23.f, offset, std::string(buf));
                        offset += 14.f;
                        std::sprintf(buf, "%1.3lf", part.solid->getVolume()*1e3);
                        gui_->DoLabel(hOffset + 18.f, offset, std::string("Volume[dm3]: ") + std::string(buf));
                    }
                }
            }
                break;
                
            default:
                break;
        }
    }
    
    //Bottom panel
    gui_->DoPanel(-10, getWindowHeight()-30.f, getWindowWidth()+20, 30.f);
    
    std::sprintf(buf, "Drawing time: %1.2lf ms (FPS %1.0lf)", getDrawingTime(), fps_);
    gui_->DoLabel(10, getWindowHeight() - 20.f, buf);
    
    std::sprintf(buf, "CPU usage: %1.0lf%%", getSimulationManager()->getCpuUsage());
    gui_->DoLabel(220, getWindowHeight() - 20.f, buf);
    
    std::sprintf(buf, "Simulation time: %1.2lf s", getSimulationManager()->getSimulationTime());
    gui_->DoLabel(350, getWindowHeight() - 20.f, buf);

    gui_->DoLabel(getWindowWidth() - 100.f, getWindowHeight() - 20.f, "Hit [K] for keymap");

    //Keymap
    if(displayKeymap_)
    {
        offset = getWindowHeight()-246.f;
        GLfloat left = getWindowWidth()-130.f; 
        gui_->DoPanel(left - 10.f, offset, 130.f, 206.f); offset += 10.f;
        gui_->DoLabel(left, offset, "[H] show/hide GUI"); offset += 16.f;
        gui_->DoLabel(left, offset, "[C] show/hide console"); offset += 16.f;
        gui_->DoLabel(left, offset, "[W] move forward"); offset += 16.f;
        gui_->DoLabel(left, offset, "[S] move backward"); offset += 16.f;
        gui_->DoLabel(left, offset, "[A] move left"); offset += 16.f;
        gui_->DoLabel(left, offset, "[D] move right"); offset += 16.f;
        gui_->DoLabel(left, offset, "[Q] move up"); offset += 16.f;
        gui_->DoLabel(left, offset, "[Z] move down"); offset += 16.f;
        gui_->DoLabel(left, offset, "[Shift] move fast"); offset += 16.f;
        gui_->DoLabel(left, offset, "[Mouse right] rotate"); offset += 16.f;
        gui_->DoLabel(left, offset, "[Mouse middle] move"); offset += 16.f;
        gui_->DoLabel(left, offset, "[Mouse scroll] zoom"); offset += 16.f;
    }

    //Performance
    if(displayPerformance_)
    {
        std::vector<std::vector<GLfloat> > perfData;    
        perfData.push_back(getSimulationManager()->getPerformanceMonitor().getPhysicsTimeHistory<GLfloat>(100));
        perfData.push_back(getSimulationManager()->getPerformanceMonitor().getHydrodynamicsTimeHistory<GLfloat>(100));

        id.owner = 4;
        id.item = 0;
        gui_->DoTimePlot(id, getWindowWidth()-300, getWindowHeight()-200, 290, 160, perfData, "Performance Monitor", new Scalar[2]{-1, 10000});
    }
}

void GraphicalSimulationApp::StartSimulation()
{
    SimulationApp::StartSimulation();

    if (autostep_)
    {   
        GraphicalSimulationThreadData* data = new GraphicalSimulationThreadData{*this};
        simulationThread_ = SDL_CreateThread(GraphicalSimulationApp::RunSimulation, "simulationThread", data);
    }
}

void GraphicalSimulationApp::ResumeSimulation()
{
    SimulationApp::ResumeSimulation();
    
    if (autostep_)
    {
        GraphicalSimulationThreadData* data = new GraphicalSimulationThreadData{*this};
        simulationThread_ = SDL_CreateThread(GraphicalSimulationApp::RunSimulation, "simulationThread", data);
    }
}

void GraphicalSimulationApp::StopSimulation()
{
    SimulationApp::StopSimulation();
	selectedEntity_ = std::make_pair(nullptr, -1);
	trackballCenter_ = nullptr;
    
    if (autostep_ && simulationThread_ != nullptr)
    {
        int status;
        SDL_WaitThread(simulationThread_, &status);
        simulationThread_ = nullptr;
    }

    physicsThreadPool_.reset();
}

void GraphicalSimulationApp::StepSimulation()
{
    SimulationApp::StepSimulation();

    if(getGLPipeline()->isDrawingQueueEmpty())
    {
        SDL_LockMutex(getGLPipeline()->getDrawingQueueMutex());
        getSimulationManager()->UpdateDrawingQueue();
        SDL_UnlockMutex(getGLPipeline()->getDrawingQueueMutex());
    }
}

void GraphicalSimulationApp::CleanUp()
{
    SimulationApp::CleanUp();
    glDeleteQueries(2, timeQuery_);

    if(joystick_ != nullptr)
        SDL_JoystickClose(0);
    
    if(glLoadingContext_ != nullptr)
        SDL_GL_DeleteContext(glLoadingContext_);
    
    SDL_GL_DeleteContext(glMainContext_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

int GraphicalSimulationApp::RenderLoadingScreen(void* data)
{
    //Get application
    GraphicalSimulationApp& app = static_cast<GraphicalSimulationThreadData*>(data)->app;
    
    //Make drawing in this thread possible
    SDL_GL_MakeCurrent(app.window_, app.glLoadingContext_);  
    
    //Render loading screen
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glScissor(0, 0, app.windowW_, app.windowH_);
    glViewport(0, 0, app.windowW_, app.windowH_);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    
    while(app.loading_)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        
        //Lock to prevent adding lines to the console while rendering
        SDL_LockMutex(app.console_->getLinesMutex());
        static_cast<OpenGLConsole*>(app.console_.get())->Render(false);
        SDL_UnlockMutex(app.console_->getLinesMutex());
        
        SDL_GL_SwapWindow(app.window_);
    }
    
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    
    //Detach thread from GL context
    SDL_GL_MakeCurrent(app.window_, nullptr);

    // Destroy thread data to avoid memory leak
    delete static_cast<GraphicalSimulationThreadData*>(data);

    return 0;
}

int GraphicalSimulationApp::RunSimulation(void* data)
{
    GraphicalSimulationApp& simApp = static_cast<GraphicalSimulationThreadData*>(data)->app;
    SimulationManager* simManager = simApp.getSimulationManager();

    simManager->setCallSimulationStepCompleted(simApp.timeStep_ == Scalar(0));

    while(simApp.getState() == SimulationState::RUNNING)
    {
        simApp.StepSimulation();
    }

    // Destroy thread data to avoid memory leak
    delete static_cast<GraphicalSimulationThreadData*>(data);

    return 0;
}

}
