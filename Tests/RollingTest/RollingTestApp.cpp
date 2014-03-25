//
//  RollingTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "RollingTestApp.h"

#include "OpenGLTrackball.h"

RollingTestApp::RollingTestApp(int width, int height, RollingTestManager* sim) : SimulationApp("Rolling Test", width, height, sim)
{
}

void RollingTestApp::MouseDown(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseDown(xPos, yPos);
}

void RollingTestApp::MouseUp(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseUp();
}

void RollingTestApp::MouseMove(SDL_Event* event)
{
    GLfloat xPos = (GLfloat)(event->motion.x-getWindowWidth()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    GLfloat yPos = -(GLfloat)(event->motion.y-getWindowHeight()/2.f)/(GLfloat)(getWindowHeight()/2.f);
    
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseMove(xPos, yPos);
}

void RollingTestApp::MouseScroll(SDL_Event* event)
{
    OpenGLTrackball* trackball = (OpenGLTrackball*)getSimulationManager()->getView(0);
    trackball->MouseScroll(event->wheel.y * -10.f);
}