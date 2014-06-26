//
//  RollingTestApp.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "RollingTestApp.h"

#include "OpenGLTrackball.h"
#include "NativeDialog.h"

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
    trackball->MouseScroll(event->wheel.y * -1.f);
}

void RollingTestApp::DoHUD()
{
    SimulationApp::DoHUD();
    
    ui_id plot;
    plot.owner = 1;
    plot.item = 0;
    plot.index = 0;
    
    std::vector<unsigned short> dims;
    dims.push_back(0);
    dims.push_back(1);
    
    if(IMGUI::getInstance()->DoTimePlot(plot, getWindowWidth()-310, 10, 300, 200, getSimulationManager()->getSensor("EncoderWheel"), dims, "Enc"))
    {
         NativeDialog* openDialog = new NativeDialog(DialogType_Save, "Save plot data...", "txt");
        openDialog->Show();
     
        char* pathToFile;
        if(openDialog->GetInput(&pathToFile) == DialogResult_OK)
        {
            const std::deque<Sample*>& data = getSimulationManager()->getSensor("EncoderWheel")->getHistory();
            if(data.size() > 0)
            {
                FILE* fp;
                fp = fopen(pathToFile, "wt");
                
                for(int i=0; i<data.size(); i++)
                    fprintf(fp, "%1.6f\n", data[i]->getValue(1));
                fclose(fp);
     
                printf("Saved plot data to %s.\n", pathToFile);
            }
        }
     
     delete [] pathToFile;
     delete openDialog;
     }
}