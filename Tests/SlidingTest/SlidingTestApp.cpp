//
//  SlidingTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestApp.h"

#include "OpenGLTrackball.h"
#include "NativeDialog.h"

SlidingTestApp::SlidingTestApp(int width, int height, SlidingTestManager* sim) : SimulationApp("Sliding Test", width, height, sim)
{
    setSimulationSpeed(1.0);
}

void SlidingTestApp::MouseDown(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseDown(xPos, yPos);
}

void SlidingTestApp::MouseUp(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseUp();
}

void SlidingTestApp::MouseMove(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseMove(xPos, yPos);
}

void SlidingTestApp::MouseScroll(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseScroll(event->wheel.y * -1.f);
}

void SlidingTestApp::DoHUD()
{
    SimulationApp::DoHUD();
    
    ui_id plot;
    plot.owner = 1;
    plot.item = 0;
    plot.index = 0;
  
    std::vector<unsigned short> dims;
    dims.push_back(2);
    IMGUI::getInstance()->DoTimePlot(plot, getWindowWidth()-310, 10, 300, 200, getSimulationManager()->getSensor(0), dims, "Height");
}
